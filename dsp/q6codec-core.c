/* Copyright (c) 2019-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2025, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/jiffies.h>
#include <linux/rpmsg.h>
#include <dsp/q6codec-core.h>
#include <linux/platform_device.h>
#include <linux/string.h>

#define Q6_READY_TIMEOUT_MS 		1000
#define CODEC_CMD_GET_CODEC_STATUS 	0x1001
#define ADD_CHILD_DEVICES_Q6_TIMEOUT_MS 300000

struct q6codec_core {
	struct rpmsg_device *adev;
	wait_queue_head_t wait;
	struct mutex lock;
	bool resp_received;
	int32_t status;
};

struct q6codec_core_private {
	struct device *dev;
	bool is_initial_boot;
	struct work_struct add_chld_dev_work;
	struct mutex lock;
	struct q6codec_core *q6codec_core_drv;
};


static struct q6codec_core_private *q6codec_core_priv;

/* used to decode basic responses from Q6 codec*/
struct q6codec_cmd_basic_rsp {
	uint32_t opcode;
	int32_t status;
	uint32_t resp_size;
	uint8_t resp_data[0];
};

/* used to send command to Q6 codec*/
struct q6codec_cmd_payload {
	uint32_t cmd_opcode;
	void *cmd_payload;
};

static int q6codec_core_callback(struct rpmsg_device *adev, void *data,
			int len, void *priv, u32 addr__unused)
{
	struct q6codec_core *core = dev_get_drvdata(&adev->dev);
	struct q6codec_cmd_basic_rsp *basic_rsp;

	basic_rsp = (struct q6codec_cmd_basic_rsp *)data;

	if (len <= 0) {
		dev_info_ratelimited(&adev->dev,
			"%s: ignoring read with len(%d) \n",
				__func__, len);
		return -EINVAL;
	}

	dev_info_ratelimited(&adev->dev, "%s: Payload %x", __func__, basic_rsp->opcode);
	switch (basic_rsp->opcode) {
	case CODEC_CMD_GET_CODEC_STATUS:
		core->status = basic_rsp->status;
		core->resp_received = true;
		dev_info_ratelimited(&adev->dev, "%s: codec status received: %d", __func__, core->status);
		break;
	default:
		dev_err_ratelimited(&adev->dev, "Message ID from q6: 0x%x\n",
			basic_rsp->opcode);
		break;
	}
	if (core->resp_received)
		wake_up(&core->wait);

	return 0;
}

static bool __q6codec_core_is_q6_ready(struct q6codec_core *core)
{
	struct rpmsg_device *adev = core->adev;
	struct q6codec_cmd_payload cmd_payload;
	int rc;
	bool ret = false;

	cmd_payload.cmd_opcode = CODEC_CMD_GET_CODEC_STATUS;

	dev_err_ratelimited(q6codec_core_priv->dev, "%s: send_command ret\n", __func__);

	rc = rpmsg_send(adev->ept, &cmd_payload, sizeof(cmd_payload));
	if (rc < 0) {
		ret = false;
		goto done;
	}

	rc = wait_event_timeout(core->wait, (core->resp_received),
				msecs_to_jiffies(Q6_READY_TIMEOUT_MS));
	dev_err_ratelimited(q6codec_core_priv->dev, "%s: wait event unblocked \n", __func__);

	if (rc > 0 && core->resp_received) {
		ret = !core->status;
		dev_err_ratelimited(q6codec_core_priv->dev, "%s: status : %d\n", __func__, ret);
	} else {
		dev_err_ratelimited(q6codec_core_priv->dev, "%s: command timedout, ret\n",
			__func__);
		core->resp_received = false;
	}

done:
	core->resp_received = false;
	return ret;
}

/**
 * q6codec_core_is_q6_ready() - Get status of adsp
 *
 * Return: Will return true if q6 is ready and false if not.
 */
bool q6codec_core_is_q6_ready(int timeout_ms)
{
	unsigned long  timeout;
	bool ret = false;
	struct q6codec_core *core;

	if (!q6codec_core_priv)
		return ret;

	mutex_lock(&q6codec_core_priv->lock);
	core = q6codec_core_priv->q6codec_core_drv;
	if (!core)
		goto done;

	timeout = jiffies + msecs_to_jiffies(timeout_ms);
	mutex_lock(&core->lock);
	for (;;) {
		if (__q6codec_core_is_q6_ready(core)) {
			ret = true;
			break;
		}
		if (!timeout_ms)
			break;

		usleep_range(50000, 50050);
		if (!time_after(timeout, jiffies)) {
			ret = false;
			break;
		}
	}

	mutex_unlock(&core->lock);
done:
	mutex_unlock(&q6codec_core_priv->lock);
	return ret;
}
EXPORT_SYMBOL_GPL(q6codec_core_is_q6_ready);

static int q6codec_core_probe(struct rpmsg_device *adev)
{
	struct q6codec_core *core;
	pr_err("%s",__func__);
	if (!q6codec_core_priv) {
		pr_err("%s: q6codec_core platform probe not yet done\n", __func__);
		return -EPROBE_DEFER;
	}
	mutex_lock(&q6codec_core_priv->lock);
	core = kzalloc(sizeof(*core), GFP_KERNEL);
	if (!core) {
		mutex_unlock(&q6codec_core_priv->lock);
		return -ENOMEM;
	}

	core->adev = adev;
	core->adev->ept = adev->ept;

	dev_set_drvdata(&adev->dev, core);

	mutex_init(&core->lock);
	init_waitqueue_head(&core->wait);
	q6codec_core_priv->q6codec_core_drv = core;
	if (q6codec_core_priv->is_initial_boot)
		schedule_work(&q6codec_core_priv->add_chld_dev_work);
	mutex_unlock(&q6codec_core_priv->lock);

	return 0;
}

static void q6codec_core_exit(struct rpmsg_device *adev)
{
	struct q6codec_core *core = dev_get_drvdata(&adev->dev);
	if (!q6codec_core_priv) {
		pr_err("%s: q6codec_core platform probe not yet done\n", __func__);
		return;
	}
	mutex_lock(&q6codec_core_priv->lock);
	q6codec_core_priv->q6codec_core_drv = NULL;
	kfree(core);
        mutex_unlock(&q6codec_core_priv->lock);
	return;
}

static const struct rpmsg_device_id qcom_q6codec_core_id_table[] = {
	{ "audio_cdc_apps_cc" },
	{}
};

static struct rpmsg_driver qcom_q6codec_core_driver = {
	.probe = q6codec_core_probe,
	.remove = q6codec_core_exit,
	.callback = q6codec_core_callback,
	.id_table = qcom_q6codec_core_id_table,
	.drv = {
		.name = "qcom_q6codec_core_ipc",
	},
};

static void q6codec_core_add_child_devices(struct work_struct *work)
{
	int ret = 0;
        pr_err("%s:enumarate machine driver\n", __func__);

	if (q6codec_core_is_q6_ready(ADD_CHILD_DEVICES_Q6_TIMEOUT_MS)) {
		dev_err(q6codec_core_priv->dev, "%s: q6 codec is up\n",
			__func__);
	} else {
		dev_err(q6codec_core_priv->dev, "%s: q6 codec is not up\n",
			__func__);
		return;
	}

	dev_err(q6codec_core_priv->dev, "%s: node found: %s\n", __func__, q6codec_core_priv->dev->of_node->name);

	ret = of_platform_populate(q6codec_core_priv->dev->of_node, NULL, NULL, q6codec_core_priv->dev);
	if (ret)
		dev_err(q6codec_core_priv->dev, "%s: failed to add child nodes, ret=%d\n",
			__func__, ret);

	dev_err(q6codec_core_priv->dev, "%s: add child nodes, done\n",
			__func__);
	q6codec_core_priv->is_initial_boot = false;

}

static int q6codec_core_platform_driver_probe(struct platform_device *pdev)
{
	int ret = 0;
        pr_err("%s",__func__);

	q6codec_core_priv = devm_kzalloc(&pdev->dev, sizeof(struct q6codec_core_private), GFP_KERNEL);
	if (!q6codec_core_priv)
		return -ENOMEM;

	q6codec_core_priv->dev = &pdev->dev;

	mutex_init(&q6codec_core_priv->lock);

	INIT_WORK(&q6codec_core_priv->add_chld_dev_work, q6codec_core_add_child_devices);

	dev_err(q6codec_core_priv->dev, "%s: node found: %s\n", __func__, q6codec_core_priv->dev->of_node->name);
	q6codec_core_priv->is_initial_boot = true;

	ret = register_rpmsg_driver(&qcom_q6codec_core_driver);
	if (ret) {
		pr_err("%s: rpmsg driver register failed = %d\n",
			__func__, ret);
		goto q6codec_core_err;
	} else {
		pr_err("%s: rpmsg driver register success = %d\n",
			__func__, ret);
	}

	return ret;
q6codec_core_err:
	cancel_work_sync(&q6codec_core_priv->add_chld_dev_work);
	mutex_destroy(&q6codec_core_priv->lock);
	devm_kfree(&pdev->dev, q6codec_core_priv);
	q6codec_core_priv = NULL;

	return ret;
}

static void q6codec_core_platform_driver_remove(struct platform_device *pdev)
{
	unregister_rpmsg_driver(&qcom_q6codec_core_driver);
	cancel_work_sync(&q6codec_core_priv->add_chld_dev_work);
	mutex_destroy(&q6codec_core_priv->lock);
	devm_kfree(&pdev->dev, q6codec_core_priv);
	q6codec_core_priv = NULL;
}

static const struct of_device_id q6codec_core_of_match[]  = {
	{ .compatible = "qcom,q6codec-core-platform", },
	{},
};

static struct platform_driver q6codec_core_driver = {
	.probe = q6codec_core_platform_driver_probe,
	.remove = q6codec_core_platform_driver_remove,
	.driver = {
		.name = "q6codec-core-platform",
		.owner = THIS_MODULE,
		.of_match_table = q6codec_core_of_match,
	}
};

module_platform_driver(q6codec_core_driver);

MODULE_DESCRIPTION("q6codec core");
MODULE_LICENSE("GPL v2");
