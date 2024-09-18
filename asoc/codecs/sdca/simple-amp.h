// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef SIMPLE_AMP_H
#define SIMPLE_AMP_H

#include <linux/gpio/consumer.h>
#include <linux/regulator/consumer.h>
#include <soc/soundwire.h>
#include <linux/debugfs.h>
#include <linux/soundwire/sdw_registers.h>

#define SIMPLE_AMP_MAX_REGISTER 0x000000
#define MCLK_12P288MHZ 12288000
#define MCLK_9P6MHZ 9600000
#define MAX_INIT_REGS 32

enum sdca_function_type {
	STEREO_FUNCTION = 0,
	MONO_CH1_FUNCTION,
	MONO_CH2_FUNCTION,
	MAX_FUNCTION,
};

#define SIMPLE_AMP_MAX_SWR_PORTS   6
#define SUPPLIES_NUM 2
#define SDCA_INTERRUPT_REG_MAX 3

/* sample frequency index */
#define SIMPLE_AMP_RATE_8000HZ          0x00
#define SIMPLE_AMP_RATE_16000HZ         0x01
#define SIMPLE_AMP_RATE_32000HZ         0x02
#define SIMPLE_AMP_RATE_44100HZ         0x03
#define SIMPLE_AMP_RATE_48000HZ         0x04
#define SIMPLE_AMP_RATE_96000HZ         0x05
#define SIMPLE_AMP_RATE_192000HZ        0x06

/* SIMPLE AMP SDCA control */
#define SIMPLE_AMP_PDE_REQ_PS              0x01
#define SIMPLE_AMP_CTL_FU_MUTE             0x01
#define SIMPLE_AMP_CTL_FU_VOLUME           0x02
#define SIMPLE_AMP_CTL_IT_USAGE            0x04
#define SIMPLE_AMP_CTL_SAMPLE_FREQ_INDEX   0x10
#define SIMPLE_AMP_CTL_CLUSTER_INDEX       0x10
#define SIMPLE_AMP_CTL_POSTURE_NUM         0x10
#define SIMPLE_AMP_CTL_PDE_ACT_PS          0x10


#define INT_DISABLE_MASK BIT(3)
#define PA0_OCP_INT_MASK BIT(4)
#define PA1_OCP_INT_MASK BIT(5)

#define PA0_FSM_ERROR_INT_HANDLE_MASK BIT(5)
#define PA1_FSM_ERROR_INT_HANDLE_MASK BIT(6)
#define POWER_FSM_ERROR_INT_HANDLE_MASK BIT(7)

#define STEREO_PROTECTION_MODE_CHANGED_HANDLE_MASK BIT(5)
#define STEREO_PLAYBACK_CLOCK_VALID_MASK  BIT(6)
#define STEREO_SENSE_CLOCK_VALID_HANDLE_MASK BIT(7)


struct simple_amp_priv {
	struct regmap *regmap;
	struct device *dev;
	struct swr_device *swr_slave;
	struct snd_soc_component *component;
	const struct snd_soc_component_driver *driver;
	struct snd_soc_dai_driver *dai_driver;
	struct sdca_function *sdca_func_data[MAX_FUNCTION];
	struct cdc_regulator *regulator;
	struct swr_port_params simple_amp_port_params[SWR_UC_MAX][SIMPLE_AMP_MAX_SWR_PORTS];
	struct swr_dev_frame_config swr_simple_amp_port_params[SWR_UC_MAX];
	struct regulator_bulk_data supplies[SUPPLIES_NUM];
	struct gpio_desc *rst_gpio_np;
	unsigned long clk_freq;
	int simple_amp_usage_mode;

	struct device_node *parent_np;
	struct platform_device *parent_dev;
	struct notifier_block parent_nblock;
	void *handle;
	int (*register_notifier)(void *handle,
				struct notifier_block *nblock, bool enable);
#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs_dent;
	struct dentry *debugfs_peek;
	struct dentry *debugfs_poke;
	struct dentry *debugfs_reg_dump;
	unsigned int read_data;
#endif
};

#endif /* SIMPLE_AMP_H */
