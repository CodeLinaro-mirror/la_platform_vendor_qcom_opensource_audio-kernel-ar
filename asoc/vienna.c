// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2025, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/of_device.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/info.h>
#include <soc/snd_event.h>
#include <dsp/spf-core.h>
#include <dsp/audio_notifier.h>
#include "msm-audio-defs.h"
#include "msm_common.h"
#include "msm_dailink.h"

#define DRV_NAME "vienna-asoc-snd"
#define __CHIPSET__ "VIENNA "
#define MSM_DAILINK_NAME(name) (__CHIPSET__#name)

enum dsp_state {
	DSP_ERROR,
	DSP_READY,
};

struct card_status {
	int state_counter;
	int prev_state_counter;
	struct mutex lock;
};

struct msm_asoc_mach_data {
	struct snd_info_entry *codec_root;
	struct msm_common_pdata *common_pdata;
	struct notifier_block notifier_adsp_nb;
	struct card_status cs;
	bool is_standby_mode_supported;
};

static const struct snd_pcm_hardware dummy_dma_hardware = {
	/* Random values to keep userspace happy when checking constraints */
	.info               = SNDRV_PCM_INFO_INTERLEAVED |
					SNDRV_PCM_INFO_BLOCK_TRANSFER,
	.buffer_bytes_max   = 128*1024,
	.period_bytes_min   = PAGE_SIZE,
	.period_bytes_max   = PAGE_SIZE*2,
	.periods_min        = 2,
	.periods_max        = 128,
};

static const char *get_domain_str(int domain)
{
	const char *domain_name = NULL;

	switch (domain) {
	case AUDIO_NOTIFIER_ADSP_DOMAIN:
		domain_name = "ADSP";
		break;
	default:
		domain_name = "UNKNOWN";
		break;
	}

	return domain_name;
}

static const char *get_opcode_str(int opcode)
{
	const char *notifier_state = NULL;

	switch (opcode) {
	case AUDIO_NOTIFIER_SERVICE_DOWN:
		notifier_state = "DOWN";
		break;
	case AUDIO_NOTIFIER_SERVICE_UP:
		notifier_state = "UP";
		break;
	default:
		notifier_state = "UNKNOWN";
		break;
	}

	return notifier_state;
}

static const char *get_snd_card_state_str(int cs)
{
	const char *card_state = NULL;

	switch (cs) {
	case SND_CARD_STATUS_OFFLINE:
		card_state = "OFFLINE";
		break;
	case SND_CARD_STATUS_ONLINE:
		card_state = "ONLINE";
		break;
	case SND_CARD_STATUS_STANDBY:
		card_state = "STANDBY";
		break;
	default:
		card_state = "INVALID";
		break;
	}

	return card_state;
}

static int msm_vienna_dai_link_init(struct snd_soc_pcm_runtime *rtd)
{
        return 0;
}

static int msm_populate_dai_link_component_of_node(
					struct snd_soc_card *card)
{
	int i, j, index, ret = 0;
	struct device *cdev = card->dev;
	struct snd_soc_dai_link *dai_link = card->dai_link;
	struct device_node *np = NULL;

	if (!cdev) {
		dev_err(cdev, "%s: Sound card device memory NULL\n", __func__);
		return -ENODEV;
	}

	for (i = 0; i < card->num_links; i++) {
		if (dai_link[i].init == NULL)
			dai_link[i].init = &msm_vienna_dai_link_init;

		/* populate codec_of_node for snd card dai links */
		if (dai_link[i].num_codecs > 0) {
			for (j = 0; j < dai_link[i].num_codecs; j++) {
				if (dai_link[i].codecs[j].of_node ||
						!dai_link[i].codecs[j].name)
					continue;

				index = of_property_match_string(cdev->of_node,
						"asoc-codec-names",
						dai_link[i].codecs[j].name);
				if (index < 0)
					continue;
				np = of_parse_phandle(cdev->of_node,
						      "asoc-codec",
						      index);
				if (!np) {
					dev_err(cdev, "%s: retrieving phandle for codec %s failed\n",
					 __func__, dai_link[i].codecs[j].name);
					ret = -ENODEV;
					goto err;
				}
				dai_link[i].codecs[j].of_node = np;
				dai_link[i].codecs[j].name = NULL;
			}
		}
	}

err:
	return ret;
}

static int msm_audrx_stub_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

static int msm_snd_stub_startup(struct snd_pcm_substream *substream)
{
        int ret = 0;
        struct snd_soc_pcm_runtime *rtd = substream->private_data;
        struct snd_soc_card *card = rtd->card;
        struct msm_common_pdata *pdata = msm_common_get_pdata(card);

        dev_dbg(rtd->card->dev,
                "%s: substream = %s  stream = %d\n",
                __func__, substream->name, substream->stream);

        if (!pdata) {
                dev_err(rtd->card->dev, "%s: pdata is NULL\n", __func__);
                return -EINVAL;
        }

        if (!rtd->dai_link->no_pcm)
                snd_soc_set_runtime_hwparams(substream, &dummy_dma_hardware);

        return ret;
}

static int msm_snd_stub_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params)
{
	return 0;
}

static struct snd_soc_ops msm_stub_be_ops = {
	.startup   = msm_snd_stub_startup,
	.hw_params = msm_snd_stub_hw_params,
};

struct snd_soc_card snd_soc_card_stub_msm = {
	.name		= "vienna-snd-card",
};

static struct snd_soc_dai_link msm_stub_be_dai_links[] = {
	/* Backend DAI Links */
	{
		.name = LPASS_BE_PRI_AUXPCM_RX,
		.stream_name = LPASS_BE_PRI_AUXPCM_RX,
		.dpcm_playback = 1,
		.init = &msm_audrx_stub_init,
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_stub_be_ops,
		SND_SOC_DAILINK_REG(auxpcm_rx),
	},
	{
		.name = LPASS_BE_PRI_AUXPCM_TX,
		.stream_name = LPASS_BE_PRI_AUXPCM_TX,
		.dpcm_playback = 1,
		.init = &msm_audrx_stub_init,
		.ignore_pmdown_time = 1,
		.ignore_suspend = 1,
		.ops = &msm_stub_be_ops,
		SND_SOC_DAILINK_REG(auxpcm_tx),
	},
};

static struct snd_soc_dai_link msm_stub_dai_links[
			 ARRAY_SIZE(msm_stub_be_dai_links)];

static const struct of_device_id vienna_asoc_machine_of_match[]  = {
	{ .compatible = "qcom,vienna-asoc-snd",
	  .data = "stub_codec"},
	{},
};

static int msm_snd_card_vienna_late_probe(struct snd_soc_card *card)
{
	struct snd_soc_pcm_runtime *rtd = NULL;

	rtd = snd_soc_get_pcm_runtime(card, &card->dai_link[0]);
	if (!rtd) {
		dev_err(card->dev,
			"%s: snd_soc_get_pcm_runtime for %s failed!\n",
			__func__, card->dai_link[0].name);
		return -EINVAL;
	}

	return 0;
}

static struct snd_soc_card *populate_snd_card_dailinks(struct device *dev)
{
	struct snd_soc_card *card = NULL;
	struct snd_soc_dai_link *dailink = NULL;
	int total_links = 0;
	const struct of_device_id *match;

	match = of_match_node(vienna_asoc_machine_of_match, dev->of_node);
	if (!match) {
		dev_err(dev, "%s: No DT match found for sound card\n",
			__func__);
		return NULL;
	}

	if(!strcmp(match->data, "stub_codec")) {
                card = &snd_soc_card_stub_msm;
                memcpy(msm_stub_dai_links,
                       msm_stub_be_dai_links,
                       sizeof(msm_stub_be_dai_links));
                total_links = ARRAY_SIZE(msm_stub_be_dai_links);

                dailink = msm_stub_dai_links;
        }

        if (card) {
                card->dai_link = dailink;
                card->num_links = total_links;
		card->late_probe = msm_snd_card_vienna_late_probe;
        }

        return card;
}

static void vienna_update_snd_card_status(struct msm_asoc_mach_data *pdata,
								int domain, int opcode,
								bool is_standby_mode_supported)
{
	int cur_state = SND_CARD_STATUS_INVALID;

	pr_debug("%s: Subsys-domain %s(%d), Service-opcode %s(%d)\n", __func__,
			get_domain_str(domain), domain, get_opcode_str(opcode), opcode);

	if (!pdata)
		return;

	mutex_lock(&pdata->cs.lock);
	switch (opcode) {
	case AUDIO_NOTIFIER_SERVICE_DOWN:
		pdata->cs.state_counter--;
		/**
		 * On 1st service down(ADSP SSR) event, MSM which has companion
		 * chip, API updates soundcard status as STANDBY so that userspace
		 * handles stream accordingly.
		 */
		if (pdata->cs.state_counter == SND_CARD_STATUS_OFFLINE) {
			if (domain == AUDIO_NOTIFIER_ADSP_DOMAIN &&
				is_standby_mode_supported) {
				cur_state = SND_CARD_STATUS_STANDBY;
			} else {
				cur_state = SND_CARD_STATUS_OFFLINE;
			}
		/**
		 * On 2nd service down event(When CC goes down post ADSP SSR),
		 * API updates current card state as OFFLINE.
		 */
		} else if (pdata->cs.state_counter == SND_CARD_STATUS_INVALID) {
			if (pdata->cs.prev_state_counter == SND_CARD_STATUS_STANDBY)
				cur_state = SND_CARD_STATUS_OFFLINE;
		} else {
			break;
		}

		/* Update previous card state */
		if (cur_state == SND_CARD_STATUS_OFFLINE ||
			cur_state == SND_CARD_STATUS_STANDBY) {
			pdata->cs.prev_state_counter = cur_state;
			snd_card_notify_user(cur_state);
			pr_info("%s: Sound card is in %s\n", __func__,
					get_snd_card_state_str(cur_state));
		}
		break;
	case AUDIO_NOTIFIER_SERVICE_UP:
		/**
		 * Up notification comes once for ADSP and CC/CC_DSP as part
		 * of bootup. Post bootup as part of SSR, API counts ADSP and
		 * CC/CC_DSP's notification independently.
		 */
		pdata->cs.state_counter++;
		if (pdata->cs.state_counter == SND_CARD_STATUS_ONLINE) {
			snd_card_notify_user(SND_CARD_STATUS_ONLINE);
			pr_info("%s: Sound card is in ONLINE\n", __func__);
		}
		break;
	default:
		break;
	}
	mutex_unlock(&pdata->cs.lock);
}

static int vienna_adsp_notifier_service_cb(struct notifier_block *this,
					 unsigned long opcode, void *ptr)
{
	struct msm_asoc_mach_data *pdata = NULL;

	pr_debug("%s: Service opcode 0x%lx\n", __func__, opcode);

	if (!this)
		return NOTIFY_STOP;

	pdata = container_of(this, struct msm_asoc_mach_data,
							notifier_adsp_nb);

	switch (opcode) {
	case DSP_ERROR:
		vienna_update_snd_card_status(pdata, AUDIO_NOTIFIER_ADSP_DOMAIN,
								AUDIO_NOTIFIER_SERVICE_DOWN, false);
		break;
	case DSP_READY:
		vienna_update_snd_card_status(pdata, AUDIO_NOTIFIER_ADSP_DOMAIN,
								AUDIO_NOTIFIER_SERVICE_UP, false);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static int vienna_ssr_enable(struct device *dev, void *data, int domain)
{
	struct msm_asoc_mach_data *pdata = NULL;
	struct platform_device *pdev = to_platform_device(dev);
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	int ret = 0;

	if (!card) {
		dev_err(dev, "%s: card is NULL\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	pdata = snd_soc_card_get_drvdata(card);

	if (!strcmp(card->name, "vienna-stub-snd-card")) {
		/* TODO */
		dev_dbg(dev, "%s: TODO\n", __func__);
	}

	vienna_update_snd_card_status(pdata, domain, AUDIO_NOTIFIER_SERVICE_UP, false);

err:
	return ret;
}

static void vienna_ssr_disable(struct device *dev, void *data, int domain)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct msm_asoc_mach_data *pdata = NULL;

	if (!card) {
		dev_err(dev, "%s: card is NULL\n", __func__);
		return;
	}

	pdata = snd_soc_card_get_drvdata(card);

	if (pdata)
		vienna_update_snd_card_status(pdata, domain, AUDIO_NOTIFIER_SERVICE_DOWN,
							pdata->is_standby_mode_supported);

	if (!strcmp(card->name, "vienna-stub-snd-card")) {
		/* TODO */
		dev_dbg(dev, "%s: TODO\n", __func__);
	}
}

static const struct snd_event_ops_v2 vienna_ssr_ops = {
	.enable = vienna_ssr_enable,
	.disable = vienna_ssr_disable,
};

static int msm_audio_ssr_compare(struct device *dev, void *data)
{
	struct device_node *node = data;

	dev_dbg(dev, "%s: dev->of_node = 0x%p, node = 0x%p\n",
		__func__, dev->of_node, node);
	return (dev->of_node && dev->of_node == node);
}

static int msm_audio_ssr_register(struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct snd_event_clients *ssr_clients = NULL;
	struct device_node *node = NULL;
	int ret = 0;
	int i = 0;

	for (i = 0; ; i++) {
		node = of_parse_phandle(np, "qcom,msm_audio_ssr_devs", i);
		if (!node)
			break;
		snd_event_mstr_add_client(&ssr_clients,
					msm_audio_ssr_compare, node);
	}

	ret = snd_event_master_register_v2(dev, &vienna_ssr_ops,
		  ssr_clients, NULL);
	if (!ret)
		snd_event_notify(dev, SND_EVENT_UP);

	return ret;
}

struct msm_common_pdata *msm_common_get_pdata(struct snd_soc_card *card)
{
	struct msm_asoc_mach_data *pdata = snd_soc_card_get_drvdata(card);

	if (!pdata)
		return NULL;

	return pdata->common_pdata;
}

void msm_common_set_pdata(struct snd_soc_card *card,
			  struct msm_common_pdata *common_pdata)
{
	struct msm_asoc_mach_data *pdata = snd_soc_card_get_drvdata(card);

	if (!pdata)
		return;

	pdata->common_pdata = common_pdata;
}

static int msm_asoc_machine_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = NULL;
	struct msm_asoc_mach_data *pdata = NULL;
	int ret = 0;

	if (!pdev->dev.of_node) {
		dev_err(&pdev->dev, "%s: No platform supplied from device tree\n",
								__func__);
		return -EINVAL;
	}

	pdata = devm_kzalloc(&pdev->dev,
			sizeof(struct msm_asoc_mach_data), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	card = populate_snd_card_dailinks(&pdev->dev);
	if (!card) {
		dev_err(&pdev->dev, "%s: Card uninitialized\n", __func__);
		ret = -EINVAL;
		goto err;
	}

	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, pdata);

	ret = snd_soc_of_parse_card_name(card, "qcom,model");
	if (ret) {
		dev_err(&pdev->dev, "%s: parse card name failed, err:%d\n",
			__func__, ret);
		goto err;
	}

	ret = msm_populate_dai_link_component_of_node(card);
	if (ret) {
		ret = -EPROBE_DEFER;
		goto err;
	}

	ret = devm_snd_soc_register_card(&pdev->dev, card);
	if (ret) {
		dev_err(&pdev->dev, "%s: snd_soc_register_card failed (%d)\n",
			__func__, ret);
		goto err;
	}
	dev_info(&pdev->dev, "%s: Sound card %s registered\n",
		 __func__, card->name);

	mutex_init(&pdata->cs.lock);
	/**
	 * By default state is OFFLINE, once ADSP is up post registration
	 * to snd event, state_counter moves to ONLINE using
	 * vienna_update_snd_card_status API.
	 */
	pdata->cs.state_counter = SND_CARD_STATUS_OFFLINE;

	ret = msm_audio_ssr_register(&pdev->dev);
	if (ret)
		pr_err("%s: Registration with SND event FWK failed ret = %d\n",
			__func__, ret);

	pdata->notifier_adsp_nb.notifier_call  = vienna_adsp_notifier_service_cb;
	pdata->notifier_adsp_nb.priority = 0;
	return 0;
err:
	devm_kfree(&pdev->dev, pdata);
	return ret;
}

static void msm_asoc_machine_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct msm_asoc_mach_data *pdata = NULL;
	struct msm_common_pdata *common_pdata = NULL;

	if (card)
		pdata = snd_soc_card_get_drvdata(card);

	if (pdata)
	{
		common_pdata = pdata->common_pdata;
		if (common_pdata)
			msm_common_snd_deinit(common_pdata);

	}
	snd_event_master_deregister(&pdev->dev);
	if (card)
		snd_soc_unregister_card(card);

	if (pdata) {
		pdata->cs.state_counter = SND_CARD_STATUS_OFFLINE;
		mutex_destroy(&pdata->cs.lock);
	}
}

static struct platform_driver vienna_asoc_machine_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm = &snd_soc_pm_ops,
		.of_match_table = vienna_asoc_machine_of_match,
		.suppress_bind_attrs = true,
	},
	.probe = msm_asoc_machine_probe,
	.remove = msm_asoc_machine_remove,
};

static int __init msm_asoc_machine_init(void)
{
	snd_card_sysfs_init();
	return platform_driver_register(&vienna_asoc_machine_driver);
}
module_init(msm_asoc_machine_init);

static void __exit msm_asoc_machine_exit(void)
{
	platform_driver_unregister(&vienna_asoc_machine_driver);
}
module_exit(msm_asoc_machine_exit);

MODULE_DESCRIPTION("ALSA SoC msm");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRV_NAME);
MODULE_DEVICE_TABLE(of, vienna_asoc_machine_of_match);
