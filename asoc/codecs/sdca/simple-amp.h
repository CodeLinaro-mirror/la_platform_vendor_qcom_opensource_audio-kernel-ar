
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

#define MCLK_9P6MHZ 9600000
#define MAX_INIT_REGS 64
#define MAX_CNTL_NUMBERS 4
#define NUM_IMP_DEF_REG_VAL_PAIRS  16

#define MAX_SWR_SLV_PORTS 4
#define PORT_1 1
#define PORT_2 2
#define PORT_3 3
#define PORT_4 4

#define CH1 0x1
#define CH2 0x2

#define SWRS_BASE                      0x00
#define SWRS_DP_PREPARE_CONTROL(n)     (SWRS_BASE+0x5+0x100*n)
#define SWRS_SCP_COMMIT  0x4c


enum sdca_function_type {
	FUNCTION_ZERO = 0,
	FUNCTION_STEREO = 1,
	FUNCTION_MONO_CH1 = 2,
	FUNCTION_MONO_CH2 = 3,
	MAX_FUNCTION,
};

#define SIMPLE_AMP_MAX_SWR_PORTS  4
#define SUPPLIES_NUM 2
#define SDCA_INTERRUPT_REG_MAX 3

/* sample frequency index for RX */
#define SIMPLE_AMP_RX_RATE_8000HZ          0x00
#define SIMPLE_AMP_RX_RATE_16000HZ         0x01
#define SIMPLE_AMP_RX_RATE_32000HZ         0x02
#define SIMPLE_AMP_RX_RATE_44100HZ         0x03
#define SIMPLE_AMP_RX_RATE_48000HZ         0x04
#define SIMPLE_AMP_RX_RATE_96000HZ         0x05
#define SIMPLE_AMP_RX_RATE_192000HZ        0x06

/* sample frequency index for TX Sense */
#define SIMPLE_AMP_VI_RATE_8000HZ          0x00
#define SIMPLE_AMP_VI_RATE_16000HZ         0x01
#define SIMPLE_AMP_VI_RATE_44100HZ         0x02
#define SIMPLE_AMP_VI_RATE_48000HZ         0x03
#define SIMPLE_AMP_VI_RATE_96000HZ         0x04
#define SIMPLE_AMP_VI_RATE_22050HZ         0x05
#define SIMPLE_AMP_VI_RATE_24000HZ         0x06
#define SIMPLE_AMP_VI_RATE_192000HZ        0x07


/* SIMPLE AMP SDCA control selectors*/
#define SIMPLE_AMP_PDE_REQ_PS              0x01
#define SIMPLE_AMP_CTL_FU21_MUTE           0x01
#define SIMPLE_AMP_CTL_FU21_VOLUME         0x02
#define SIMPLE_AMP_CTL_CLOCK_VALID         0x02
#define SIMPLE_AMP_CTL_IT_USAGE            0x04
#define SIMPLE_AMP_CTL_SAMPLE_FREQ_INDEX   0x10
#define SIMPLE_AMP_CTL_CLUSTER_INDEX       0x10
#define SIMPLE_AMP_CTL_POSTURE_NUM         0x10
#define SIMPLE_AMP_CTL_PDE_ACT_PS          0x10


#define SDCA_INT1_MASK (SDW_SCP_SDCA_INTMASK_SDCA_3 | SDW_SCP_SDCA_INTMASK_SDCA_4 \
				| SDW_SCP_SDCA_INTMASK_SDCA_5)
#define SDCA_INT2_MASK (SDW_SCP_SDCA_INTMASK_SDCA_8 | SDW_SCP_SDCA_INTMASK_SDCA_11 \
		| SDW_SCP_SDCA_INTMASK_SDCA_13 | SDW_SCP_SDCA_INTMASK_SDCA_14 \
		| SDW_SCP_SDCA_INTMASK_SDCA_15)
#define SDCA_INT3_MASK (SDW_SCP_SDCA_INTMASK_SDCA_21 | SDW_SCP_SDCA_INTMASK_SDCA_22 \
		| SDW_SCP_SDCA_INTMASK_SDCA_23)

/* FU Volume control registers */
#define ENTITY_FU21  0x4
#define SIMPLE_AMP_FU21_CH_VOL_CH2X0    SDW_SDCA_CTL(FUNCTION_STEREO, \
				ENTITY_FU21, SIMPLE_AMP_CTL_FU21_VOLUME, CH1)
#define SIMPLE_AMP_FU21_CH_VOL_CH2X1    SDW_SDCA_CTL(FUNCTION_STEREO, \
				ENTITY_FU21, SIMPLE_AMP_CTL_FU21_VOLUME, CH2)

#define SIMPLE_AMP_FU21_CH_VOL_CH2X0_LSB   SDW_SDCA_NEXT_CTL(SIMPLE_AMP_FU21_CH_VOL_CH2X0)
#define SIMPLE_AMP_FU21_CH_VOL_CH2X0_MSB   SDW_SDCA_MBQ_CTL(SIMPLE_AMP_FU21_CH_VOL_CH2X0_LSB)
#define SIMPLE_AMP_FU21_CH_VOL_CH2X1_LSB   SDW_SDCA_NEXT_CTL(SIMPLE_AMP_FU21_CH_VOL_CH2X1)
#define SIMPLE_AMP_FU21_CH_VOL_CH2X1_MSB   SDW_SDCA_MBQ_CTL(SIMPLE_AMP_FU21_CH_VOL_CH2X1_LSB)

/* MONO CH1 - LEFT */
#define SIMPLE_AMP_FU21_LEFT_CH_VOL_CH2X0    SDW_SDCA_CTL(FUNCTION_MONO_CH1, \
			ENTITY_FU21, SIMPLE_AMP_CTL_FU21_VOLUME, CH1)

#define SIMPLE_AMP_FU21_LEFT_CH_VOL_CH2X0_LSB SDW_SDCA_NEXT_CTL(SIMPLE_AMP_FU21_LEFT_CH_VOL_CH2X0)
#define SIMPLE_AMP_FU21_LEFT_CH_VOL_CH2X0_MSB SDW_SDCA_MBQ_CTL(SIMPLE_AMP_FU21_LEFT_CH_VOL_CH2X0_LSB)

/* MONO CH2 - RIGHT */
#define SIMPLE_AMP_FU21_RIGHT_CH_VOL_CH2X1   SDW_SDCA_CTL(FUNCTION_MONO_CH2, \
			ENTITY_FU21, SIMPLE_AMP_CTL_FU21_VOLUME, CH1)

#define SIMPLE_AMP_FU21_RIGHT_CH_VOL_CH2X1_LSB SDW_SDCA_NEXT_CTL(SIMPLE_AMP_FU21_RIGHT_CH_VOL_CH2X1)
#define SIMPLE_AMP_FU21_RIGHT_CH_VOL_CH2X1_MSB SDW_SDCA_MBQ_CTL(SIMPLE_AMP_FU21_RIGHT_CH_VOL_CH2X1_LSB)

struct port_config {
	int num_ports;
	u8 port_id[MAX_SWR_SLV_PORTS];
	u8 ch_mask[MAX_SWR_SLV_PORTS];
	u8 port_type[MAX_SWR_SLV_PORTS];
	u8 num_ch[MAX_SWR_SLV_PORTS];
	unsigned int ch_rate[MAX_SWR_SLV_PORTS];
	struct swr_device *swr_slave;
};

struct port_config_dual_mono {
	struct port_config pconfig_mono_ch1;
	struct port_config pconfig_mono_ch2;
};

union port_config_sel {
	struct port_config pconfig_stereo;
	struct port_config_dual_mono dual_mono;
};

struct entity_idx_table {
    int entity_idx;
    const char *entity_label;
};

struct impl_def_reg_val_pair {
	uint32_t reg;
	uint32_t value;
};

struct simple_amp_priv {
	struct regmap *regmap;
	struct device *dev;
	struct swr_device *swr_slave;
	struct snd_soc_component *component;
	const struct snd_soc_component_driver *driver;
	struct snd_soc_dai_driver *dai_driver;
	struct sdca_function *sdca_func_data[MAX_FUNCTION];
	struct cdc_regulator *regulator;
	struct regulator_bulk_data supplies[SUPPLIES_NUM];
	struct gpio_desc *sd_n;
	unsigned long clk_freq;
	u32 num_functions;
	int usage_mode;
	int stereo_voldB;
	int ch1_voldB;
	int rx_ch_sel;
	bool debug_mode_enable;

	struct device_node *parent_np;
	struct platform_device *parent_dev;
	struct notifier_block parent_nblock;
	void *handle;
	int (*register_notifier)(void *handle,
				struct notifier_block *nblock, bool enable);
	struct impl_def_reg_val_pair reg_val_pair[NUM_IMP_DEF_REG_VAL_PAIRS];
	size_t num_pairs;
	union port_config_sel pconfig;
#ifdef CONFIG_DEBUG_FS
	struct dentry *debugfs_dent;
	struct dentry *debugfs_peek;
	struct dentry *debugfs_poke;
	struct dentry *debugfs_reg_dump;
	unsigned int read_data;
#endif
};

#endif /* SIMPLE_AMP_H */
