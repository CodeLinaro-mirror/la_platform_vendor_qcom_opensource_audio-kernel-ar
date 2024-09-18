// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/mod_devicetable.h>
#include <linux/of_platform.h>
#include <linux/pm_runtime.h>
#include <linux/slab.h>
#include <linux/qti-regmap-debugfs.h>
#include <linux/regmap.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include "asoc/bolero-slave-internal.h"
#include "simple-amp.h"

#define SIMPLE_AMP_RATES (SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
		SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000)
#define SIMPLE_AMP_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE | \
		SNDRV_PCM_FMTBIT_S32_LE)

#ifdef CONFIG_DEBUG_FS
extern void simple_amp_regdump_register(struct simple_amp_priv *simple_amp,
	struct swr_device *pdev);
#endif

enum sdca_entity_type {
	SDCA_ENTITY_TYPE_IT = 0x02,	/* Input Terminal */
	SDCA_ENTITY_TYPE_OT,		/* Output Terminal */
	SDCA_ENTITY_TYPE_MU = 0x05,	/* Mixer Unit */
	SDCA_ENTITY_TYPE_SU,		/* Selector Unit */
	SDCA_ENTITY_TYPE_FU,		/* Feature Unit */
	SDCA_ENTITY_TYPE_XU = 0x0A,	/* Extension Unit */
	SDCA_ENTITY_TYPE_CS,		/* Clock Source */
	SDCA_ENTITY_TYPE_CX,		/* Clock selector */
	SDCA_ENTITY_TYPE_PDE = 0x11,	/* Power-Domain Entity */
	SDCA_ENTITY_TYPE_GE,		/* Group Entity */
	SDCA_ENTITY_TYPE_PCE,		/* Privacy Control Entity */
	SDCA_ENTITY_TYPE_CRU = 0x20,	/* Channel Remapping Unit */
	SDCA_ENTITY_TYPE_UDMPU,		/* UpDownMixerUnit */
	SDCA_ENTITY_TYPE_MFPU,		/* Multi-Function Processing Unit */
	SDCA_ENTITY_TYPE_SMPU,		/* Smart Mic Processing Unit */
	SDCA_ENTITY_TYPE_SAPU,		/* Smart Amp Processing Unit */
	SDCA_ENTITY_TYPE_TG = 0x30,	/* Tone Generator */
	SDCA_ENTITY_TYPE_HIDE		/* HID Entity */
};

/*
 *	Bits		Contents
 *	31		0 (required by addressing range)
 *	30:26		0b10000 (Control Prefix)
 *	25		0 (Reserved)
 *	24:22		Function Number [2:0]
 *	21		Entity[6]
 *	20:19		Control Selector[5:4]
 *	18		0 (Reserved)
 *	17:15		Control Number[5:3]
 *	14		Next
 *	13		MBQ
 *	12:7		Entity[5:0]
 *	6:3		Control Selector[3:0]
 *	2:0		Control Number[2:0]
*/

#define FUN_MASK  GENMASK(24, 22)
#define ENT_MASK1 BIT(21)
#define ENT_MASK2 GENMASK(12, 7)
#define CTL_SEL_MASK1 GENMASK(22, 19)
#define CTL_SEL_MASK2 GENMASK(6, 3)
#define CH_MASK   GENMASK(2, 0)

#define SDW_SDCA_EXTRACT_ALL(reg, fun, ent, ctl_sel, ch) \
	do { \
		(fun) = FIELD_GET(FUN_MASK, reg); \
		(ent) = (FIELD_GET(ENT_MASK1, reg) << 6) | FIELD_GET(ENT_MASK2, reg); \
		(ctl_sel) = (FIELD_GET(CTL_SEL_MASK1, reg) << 4) | FIELD_GET(CTL_SEL_MASK2, reg); \
		(ch) = FIELD_GET(CH_MASK, reg); \
	} while (0)

static const char * const supply_name[] = {
	"vdd-io",
	"vdd-1p8",
};

enum num_entity {
	ENTITY_TYPE_0,
	ENTITY_TYPE_IT_21,
	ENTITY_TYPE_CS_21,
	ENTITY_TYPE_PPU_21,
	ENTITY_TYPE_FU_21,
	ENTITY_TYPE_SAPU_29,
	ENTITY_TYPE_PDE_23,
	ENTITY_TYPE_OT_23,
	ENTITY_TYPE_CS_24,
	ENTITY_TYPE_OT_24,
	ENTITY_TYPE_TG_23,
	ENTITY_TYPE_MAX,
};


enum access_mode {
	READ_ONLY = 0,
	WRITE_ONLY = 1,
	READ_WRITE = 2,
	INVALID_MODE,
};

void int_disable_handle(void);
void pa0_ocp_int_handle(void);
void pa1_ocp_int_handle(void);
void clock_stop_int_handle(void);
void uvlo_int_handle(void);
void pa0_fsm_error_int_handle(void);
void pa1_fsm_error_int_handle(void);
void power_fsm_error_int_handle(void);
void stereo_protection_Mode_Changed_handle(void);
void stereo_playback_clock_valid_handle(void);
void stereo_sense_clock_valid_handle(void);
static bool simple_amp_readable_register(struct device *dev, unsigned int reg);


struct amp_ctrl_platform_data {
	void *handle;
	int (*update_amp_event)(void *handle, u16 event, u32 data);
	int (*register_notifier)(void *handle, struct notifier_block *nblock,
				bool enable);
};

struct sdca_control_sel_data {
	u32 access_layer;
	u32 dc_value;
	enum access_mode access_mode;
};

struct sdca_entity {
	int entity_id;
	const char *entity_label;
	enum sdca_entity_type entity_type;
	u32 terminal_type;
	u32 entity_cntrl_list;
	struct sdca_control_sel_data *control_sel;
	int num_ctl_sel;
	u32 *control_sel_list; //Array for control selector values
	struct sdca_control_sel_data **control_sel_map;
};

struct sdca_initialization_table_entry {
	u32 address;
	u32 value;
};

struct sdca_function {
	u32 function_number;
	struct sdca_entity entity_data[ENTITY_TYPE_MAX];
	struct sdca_initialization_table_entry *init_table;
	int init_table_count;
};

unsigned int get_access_mode(struct sdca_entity *entity_data, unsigned int ctl_sel)
{
	int i =0;

	if (!entity_data || !entity_data->control_sel_map)
		return INVALID_MODE;

	for (i=0; i <= entity_data->num_ctl_sel; i++) {
		if (entity_data->control_sel_list[i] == ctl_sel) {
			return entity_data->control_sel_map[ctl_sel]->access_mode;
		}
	}
	return INVALID_MODE;
}

static const struct reg_default simple_amp_reg_defaults[]  = {
};

static bool simple_amp_readable_register(struct device *dev, unsigned int reg)
{
	unsigned int fun, ent, ctl_sel, ch;
	unsigned int access_mode;
	struct sdca_entity *entity_data;

	struct simple_amp_priv *simple_amp = dev_get_drvdata(dev);

	if (!simple_amp)
		return false;

	SDW_SDCA_EXTRACT_ALL(reg, fun, ent, ctl_sel, ch);

	entity_data = &simple_amp->sdca_func_data[fun]->entity_data[ent];

	access_mode = get_access_mode(entity_data, ctl_sel);
	if ((access_mode == READ_ONLY) || (access_mode == READ_WRITE))
		return true;
	else
		return false;
}

static bool simple_amp_writeable_register(struct device *dev, unsigned int reg)
{
	unsigned int fun, ent, ctl_sel, ch;
	unsigned int access_mode;
	struct sdca_entity *entity_data;

	struct simple_amp_priv *simple_amp = dev_get_drvdata(dev);
	if (!simple_amp)
		return false;

	SDW_SDCA_EXTRACT_ALL(reg, fun, ent, ctl_sel, ch);

	entity_data = &simple_amp->sdca_func_data[fun]->entity_data[ent];

	access_mode = get_access_mode(entity_data, ctl_sel);

	if ((access_mode == WRITE_ONLY) || (access_mode == READ_WRITE))
		return true;
	else
		return false;
}

static bool simple_amp_volatile_register(struct device *dev, unsigned int reg)
{
	unsigned int fun, ent, ctl_sel, ch;
	unsigned int access_mode;
	struct sdca_entity *entity_data;

	struct simple_amp_priv *simple_amp = dev_get_drvdata(dev);
	if (!simple_amp)
		return false;

	SDW_SDCA_EXTRACT_ALL(reg, fun, ent, ctl_sel, ch);

	entity_data = &simple_amp->sdca_func_data[fun]->entity_data[ent];

	access_mode = get_access_mode(entity_data, ctl_sel);

	if (access_mode == READ_ONLY)
		return true;
	else
		return false;
}

static const struct regmap_config simple_amp_regmap = {
	.reg_bits = 32,
	.val_bits = 8,
	.readable_reg = simple_amp_readable_register,
	.volatile_reg = simple_amp_volatile_register,
	.writeable_reg = simple_amp_writeable_register,
	.max_register = SIMPLE_AMP_MAX_REGISTER,
	.reg_defaults = simple_amp_reg_defaults,
	.num_reg_defaults = ARRAY_SIZE(simple_amp_reg_defaults),
	.cache_type = REGCACHE_RBTREE,
	.use_single_read = true,
	.use_single_write = true,
};


static int parse_control_selectors(struct device *dev,
		struct device_node *control_np, struct sdca_entity *entity)
{
	struct device_node *subproperty_np;
	int ret, i = 0;
	const char *name;
	u32 control_selector_hex;
	u32 access_mode;

	entity->control_sel = devm_kzalloc(dev,
			sizeof(struct sdca_control_sel_data) * entity->num_ctl_sel,
			GFP_KERNEL);

	if (!entity->control_sel)
		return -ENOMEM;

	entity->control_sel_map = devm_kzalloc(dev,
			64 * sizeof(struct sdca_control_sel_data *), GFP_KERNEL); //Asume max 64 control selectors
	if (!entity->control_sel_map) {
		pr_err("Failed to allocate memory for control map\n");
		return -ENOMEM;
	}

	for_each_child_of_node(control_np, subproperty_np) {
		name =  subproperty_np->name;

		if (i >= entity->num_ctl_sel) {
			pr_warn("Exceeded maximum number of control subproperties\n");
			break;
		}

		ret = of_property_read_u32(subproperty_np, "mipi-sdca-control-access-layer",
				&entity->control_sel[i].access_layer);
		if (ret) {
			pr_err("Failed to read control access layer for subproperty %d\n", i);
			return ret;
		}

		ret = of_property_read_u32(subproperty_np, "mipi-sdca-control-dc-value",
				&entity->control_sel[i].dc_value);
		if (ret) {
			pr_err("Failed to read dc values subproperty %d\n", i);
			return ret;
		}

		ret = of_property_read_u32(subproperty_np, "mipi-sdca-control-access-mode",
				&access_mode);
		if (ret) {
			pr_err("Failed to read control access mode for subproperty %d\n", i);
			return ret;
		}

		switch (access_mode) {
			case 0:
				entity->control_sel[i].access_mode = READ_ONLY;
				break;
			case 1:
				entity->control_sel[i].access_mode = WRITE_ONLY;
				break;
			case 2:
				entity->control_sel[i].access_mode = READ_WRITE;
				break;
			default:
				pr_err("Unknown access mode\n");
				return -EINVAL;
		}

		control_selector_hex = entity->control_sel_list[i];

		entity->control_sel_map[control_selector_hex] = &entity->control_sel[i];
		i++;
	}

	return 0;
}

static int parse_entity(struct device *dev, struct device_node *function_node,
		struct sdca_entity *entity_data, int entity_id)
{
	struct device_node *entity_np, *control_np;
	char entity_name[32];
	int ret;

	snprintf(entity_name, sizeof(entity_name), "mipi-sdca-entity-id-0x%x", entity_id);

	entity_np = of_get_child_by_name(function_node, entity_name);
	if (!entity_np) {
		dev_err(dev, "Failed to find entity node %s\n", entity_name);
		return -EINVAL;
	}

	ret = of_property_read_u32(entity_np, "mipi-sdca-entity-type", (u32 *)&entity_data->entity_type);
	if (ret) {
		dev_err(dev, "Failed to read entity type\n");
		return ret;
	}

	entity_data->entity_label = of_get_property(entity_np, "mipi-sdca-entity-label", NULL);
	if (!entity_data->entity_label) {
		dev_err(dev, "Failed to read entity label\n");
		return -EINVAL;
	}

	ret = of_property_read_u32(entity_np, "mipi-sdca-terminal-type", &entity_data->terminal_type);
	if (ret) {
		dev_err(dev, "Failed to read terminal type\n");
		return ret;
	}

	ret = of_property_read_u32(entity_np, "mipi-sdca-control-list", &entity_data->entity_cntrl_list);
	if (ret) {
		dev_err(dev, "Failed to read control list\n");
		return ret;
	}

	/* parse the sub-node sdca_control_list */
	control_np = of_get_child_by_name(entity_np, "mipi_sdca_control_list");
	if (!control_np) {
		dev_err(dev, "Failed to find control list node\n");
		return -EINVAL;
	}

	entity_data->num_ctl_sel = of_property_count_elems_of_size(entity_np,
				"mipi-sdca-control-selectors", sizeof(u32));
	if (entity_data->num_ctl_sel < 0) {
		dev_err(dev,"%s: failed to count control selector elements\n", __func__);
		return entity_data->num_ctl_sel;
	}

	entity_data->control_sel_list = devm_kzalloc(dev,
				entity_data->num_ctl_sel * sizeof(u32), GFP_KERNEL);
	if (!entity_data->control_sel_list) {
		dev_err(dev, "%s: Failed to allocate memory for cntl_sels\n", __func__);
		return -ENOMEM;
	}

	ret = of_property_read_u32_array(entity_np, "mipi-sdca-control-selectors",
		entity_data->control_sel_list, entity_data->num_ctl_sel);
	if (ret < 0) {
		dev_err(dev, "%s: Failed to read property\n", __func__);
		kfree(entity_data->control_sel_list);
		return ret;
	}

	ret = parse_control_selectors(dev, control_np, entity_data);
	if (ret) {
		dev_err(dev, "Failed to parse control subproperties\n");
		return ret;
	}

	return 0;
}

static int parse_initialization_table(struct device *dev, struct device_node *function_node,
		struct sdca_function *sdca_func)
{
	int ret;
	int j;

	sdca_func->init_table_count = of_property_count_u32_elems(function_node,
			"mipi-sdca-function-initialization-table") / 2;
	sdca_func->init_table = devm_kzalloc(dev,
			sizeof(struct sdca_initialization_table_entry) *
			sdca_func->init_table_count, GFP_KERNEL);
	if (!sdca_func->init_table)
		return -ENOMEM;

	for (j = 0; j < sdca_func->init_table_count; j++) {
		ret = of_property_read_u32_index(function_node,
				"mipi-sdca-function-initialization-table",
				2 * j, &sdca_func->init_table[j].address);
		if (ret) {
			dev_err(dev, "Failed to get init table address\n");
			return ret;
		}
		ret = of_property_read_u32_index(function_node,
				"mipi-sdca-function-initialization-table",
				2 * j + 1, &sdca_func->init_table[j].value);
		if (ret) {
			dev_err(dev, "Failed to get init table value\n");
			return ret;
		}
	}

	return 0;
}

static int parse_functions(struct device *dev, struct device_node *function_node,
		struct sdca_function *sdca_func_data)
{
	int ret = 0;
	int i = 0;

	ret = of_property_read_u32(function_node,
			"mipi-sdca-function-number", &sdca_func_data->function_number);
	if (ret) {
		dev_err(dev, "Failed to get function number\n");
		return ret;
	}

	ret = parse_initialization_table(dev, function_node, sdca_func_data);
	if (ret)
		return ret;

	for (i = 0; i < ENTITY_TYPE_MAX; i++) {
		ret = parse_entity(dev, function_node, &sdca_func_data->entity_data[i], i);
		if (ret) {
			dev_err(dev, "Failed to Parsing for Entity: %d\n", i);
		}
	}

	return ret;
}

static void parser_sdca_data(struct device *dev, struct simple_amp_priv *simple_amp)
{
	int i;
	u32 num_function = 0;
	u32 functions[MAX_FUNCTION];
	struct device_node *np = dev->of_node;
	struct device_node *function_node;

	num_function = of_property_count_elems_of_size(dev->of_node, "qcom,simple-amp-function-array", sizeof(u32));

	if (of_property_read_u32_array(dev->of_node, "qcom,simple-amp-function-array", functions, num_function)) {
		dev_err(dev, "%s: Failed to read function array\n", __func__);
	}

	struct sdca_function **ptrArray = (struct sdca_function **)devm_kzalloc(dev, num_function * sizeof(struct sdca_function *), GFP_KERNEL);

	for (i=0; i<num_function; ++i) {
		simple_amp->sdca_func_data[functions[i]] = ptrArray[i];
	}

	for_each_child_of_node(np, function_node) {
		struct sdca_function *sdca_func_data = simple_amp->sdca_func_data[functions[i]];
		parse_functions(dev, function_node, sdca_func_data);
	}

}

static const struct snd_soc_dapm_route simple_amp_audio_map[] = {
};

static const struct snd_soc_dapm_widget simple_amp_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("IN"),
};

static int simple_amp_usage_modes_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp = snd_soc_component_get_drvdata(component);

	ucontrol->value.enumerated.item[0] = simple_amp->simple_amp_usage_mode;

	return 0;
}

static int simple_amp_usage_modes_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp = snd_soc_component_get_drvdata(component);

	simple_amp->simple_amp_usage_mode = ucontrol->value.enumerated.item[0];
	return 0;
}


static const char * const simple_amp_usage_modes_text[] = {
	"MODE0", "MODE1", "MODE2", "MODE3", "MODE4", "MODE5", "MODE6", "MODE7",
};

static const struct soc_enum simple_amp_usage_modes_enum =
SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(simple_amp_usage_modes_text), simple_amp_usage_modes_text);

static const struct snd_kcontrol_new simple_amp_snd_controls[] = {
	SOC_ENUM_EXT("Usage Mode", simple_amp_usage_modes_enum,
		simple_amp_usage_modes_get, simple_amp_usage_modes_put),
};

static int simple_amp_component_probe(struct snd_soc_component *component)
{
	struct simple_amp_priv *simple_amp = snd_soc_component_get_drvdata(component);
//	char *proc_entry_name;

	simple_amp = snd_soc_component_get_drvdata(component);

	if (!simple_amp)
		return -EINVAL;

	simple_amp->component = component;
	snd_soc_component_init_regmap(component, simple_amp->regmap);

	devm_regmap_qti_debugfs_register(simple_amp->dev, simple_amp->regmap);
	return 0;
}

static void simple_amp_component_remove(struct snd_soc_component *component)
{
	struct simple_amp_priv *simple_amp = snd_soc_component_get_drvdata(component);

	if (!simple_amp)
		return;

	devm_regmap_qti_debugfs_unregister(simple_amp->regmap);
	snd_soc_component_exit_regmap(component);
}

static const struct snd_soc_component_driver soc_codec_dev_simple_amp = {
	.name = NULL,
	.probe = simple_amp_component_probe,
	.remove = simple_amp_component_remove,
	.controls = simple_amp_snd_controls,
	.num_controls = ARRAY_SIZE(simple_amp_snd_controls),
	.dapm_widgets = simple_amp_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(simple_amp_dapm_widgets),
	.dapm_routes = simple_amp_audio_map,
	.num_dapm_routes = ARRAY_SIZE(simple_amp_audio_map),
};

static int simple_amp_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct simple_amp_priv *simple_amp = snd_soc_dai_get_drvdata(dai);
	struct sdca_function *sdca_func_data = simple_amp->sdca_func_data[dai->id];

	pr_err("%s: entity Id: %d\n", __func__, sdca_func_data->entity_data[ENTITY_TYPE_CS_21].entity_id);

	return 0;
}

static int simple_amp_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct simple_amp_priv *simple_amp =
		snd_soc_component_get_drvdata(component);
	struct sdca_function *sdca_func_data = simple_amp->sdca_func_data[dai->id];
	struct sdca_entity *entity_data = &sdca_func_data->entity_data[ENTITY_TYPE_CS_21];

	int num_channels;
	unsigned int sampling_rate;

	dev_dbg(dai->dev, "%s %s", __func__, dai->name);

	num_channels = params_channels(params);

	switch (params_rate(params)) {
	case 8000:
		sampling_rate = SIMPLE_AMP_RATE_8000HZ;
		break;
	case 16000:
		sampling_rate = SIMPLE_AMP_RATE_16000HZ;
		break;
	case 32000:
		sampling_rate = SIMPLE_AMP_RATE_32000HZ;
		break;
	case 44100:
		sampling_rate = SIMPLE_AMP_RATE_44100HZ;
		break;
	case 48000:
		sampling_rate = SIMPLE_AMP_RATE_48000HZ;
		break;
	case 96000:
		sampling_rate = SIMPLE_AMP_RATE_96000HZ;
		break;
	case 192000:
		sampling_rate = SIMPLE_AMP_RATE_192000HZ;
		break;
	default:
		dev_err(component->dev, "Rate %d is not supported\n",
			params_rate(params));
		return -EINVAL;
	}

	/* configure RX sample rate */
	regmap_write(simple_amp->regmap,
		SDW_SDCA_CTL(dai->id, entity_data->entity_id,
		SIMPLE_AMP_CTL_SAMPLE_FREQ_INDEX, 0), sampling_rate);



	/* configure VI sample rate */
	entity_data = &sdca_func_data->entity_data[ENTITY_TYPE_CS_24];
	regmap_write(simple_amp->regmap,
		SDW_SDCA_CTL(dai->id, entity_data->entity_id,
		SIMPLE_AMP_CTL_SAMPLE_FREQ_INDEX, 0), 0x4);

	/* configure usage mode */
	entity_data = &sdca_func_data->entity_data[ENTITY_TYPE_OT_23];
	regmap_write(simple_amp->regmap,
		SDW_SDCA_CTL(dai->id, entity_data->entity_id,
		SIMPLE_AMP_CTL_IT_USAGE, 0), simple_amp->simple_amp_usage_mode);

	/* configure cluster Index */
	entity_data = &sdca_func_data->entity_data[ENTITY_TYPE_IT_21];
	regmap_write(simple_amp->regmap,
		SDW_SDCA_CTL(dai->id, entity_data->entity_id,
		SIMPLE_AMP_CTL_CLUSTER_INDEX, 0), 0x1);
	return 0;
}


static const struct snd_soc_dai_ops simple_amp_dai_ops = {
	.startup = simple_amp_startup,
	.hw_params = simple_amp_hw_params,
};


static struct snd_soc_dai_driver simple_amp_dai[] = {
	{
		.name = "",
		.id = STEREO_FUNCTION,
		.playback = {
			.stream_name = "",
			.rates = SIMPLE_AMP_RATES,
			.formats = SIMPLE_AMP_FORMATS,
			.rate_max = 192000,
			.rate_min = 8000,
			.channels_min = 1,
			.channels_max = 2,
		},
		.ops = &simple_amp_dai_ops,
	},
	{
		.name = "",
		.id = MONO_CH1_FUNCTION,
		.playback = {
			.stream_name = "",
			.rates = SIMPLE_AMP_RATES,
			.formats = SIMPLE_AMP_FORMATS,
			.rate_max = 192000,
			.rate_min = 8000,
			.channels_min = 1,
			.channels_max = 2,
		},
		.ops = &simple_amp_dai_ops,
	},
	{
		.name = "",
		.id = MONO_CH2_FUNCTION,
		.playback = {
			.stream_name = "",
			.rates = SIMPLE_AMP_RATES,
			.formats = SIMPLE_AMP_FORMATS,
			.rate_max = 192000,
			.rate_min = 8000,
			.channels_min = 1,
			.channels_max = 2,
		},
		.ops = &simple_amp_dai_ops,
	},
};


static struct snd_soc_dai_driver *get_dai_driver(struct device *dev, int dev_index)
{
	struct snd_soc_dai_driver *dai_drv = NULL;

	dai_drv =  devm_kzalloc(dev,
			ARRAY_SIZE(simple_amp_dai) * sizeof(struct snd_soc_dai_driver),
			GFP_KERNEL);
	if (!dai_drv)
		return NULL;


	memcpy(dai_drv, simple_amp_dai,
			ARRAY_SIZE(simple_amp_dai) * sizeof(struct snd_soc_dai_driver));

	dai_drv[0].name = devm_kasprintf(dev, GFP_KERNEL, "simple_amp_stereo_%d", dev_index);
	dai_drv[1].name = devm_kasprintf(dev, GFP_KERNEL, "simple_amp_mono_l_%d", dev_index);
	dai_drv[2].name = devm_kasprintf(dev, GFP_KERNEL, "simple_amp_mono_r_%d",
			dev_index);
	if (!dai_drv[0].name || !dai_drv[1].name || !dai_drv[2].name)
		return NULL;

	dai_drv[0].playback.stream_name = devm_kasprintf(dev, GFP_KERNEL,
			"Simple Amp AIF%d Stereo Playback", dev_index);
	dai_drv[1].playback.stream_name = devm_kasprintf(dev, GFP_KERNEL,
			"Simple Amp AIF%d Mono_L Playback", dev_index);
	dai_drv[2].playback.stream_name = devm_kasprintf(dev, GFP_KERNEL,
			"Simple Amp AIF%d Mono_R Playback", dev_index);
	if (!dai_drv[0].playback.stream_name || !dai_drv[1].playback.stream_name ||
			!dai_drv[2].playback.stream_name)
		return NULL;

	return dai_drv;
}


static int simple_amp_suspend(struct device *dev)
{
	dev_dbg(dev, "%s: system suspend\n", __func__);
	return 0;
}

static int simple_amp_resume(struct device *dev)
{
	struct simple_amp_priv *simple_amp = dev_get_drvdata(dev);

	if (!simple_amp) {
		dev_err_ratelimited(dev, "%s: simple_amp private data is NULL\n", __func__);
		return -EINVAL;
	}
	dev_dbg(dev, "%s: system resume\n", __func__);
	return 0;
}

static int parse_port_params(struct device *dev, char *prop)
{
	u32 *dt_array, map_size, max_uc;
	int ret = 0;
	u32 cnt = 0;
	u32 i, j;
	struct swr_port_params (*map)[SWR_UC_MAX][SIMPLE_AMP_MAX_SWR_PORTS];
	struct swr_dev_frame_config (*map_uc)[SWR_UC_MAX];
	struct simple_amp_priv *simple_amp = dev_get_drvdata(dev);

	map = &simple_amp->simple_amp_port_params;
	map_uc = &simple_amp->swr_simple_amp_port_params;

	if (!of_find_property(dev->of_node, prop,
				&map_size)) {
		dev_err(dev, "missing port mapping prop %s\n", prop);
		ret = -EINVAL;
		goto err_port_map;
	}

	max_uc = map_size / (SIMPLE_AMP_MAX_SWR_PORTS * SWR_PORT_PARAMS * sizeof(u32));

	if (max_uc != SWR_UC_MAX) {
		dev_err(dev, "%s: port params not provided for all usecases\n",
				__func__);
		ret = -EINVAL;
		goto err_port_map;
	}
	dt_array = kzalloc(map_size, GFP_KERNEL);

	if (!dt_array) {
		ret = -ENOMEM;
		goto err_port_map;
	}
	ret = of_property_read_u32_array(dev->of_node, prop, dt_array,
			SIMPLE_AMP_MAX_SWR_PORTS * SWR_PORT_PARAMS * max_uc);
	if (ret) {
		dev_err(dev, "%s: Failed to read port mapping from prop %s\n",
				__func__, prop);
		goto err_pdata_fail;
	}

	for (i = 0; i < max_uc; i++) {
		for (j = 0; j < SIMPLE_AMP_MAX_SWR_PORTS; j++) {
			cnt = (i * SIMPLE_AMP_MAX_SWR_PORTS + j) * SWR_PORT_PARAMS;
			(*map)[i][j].offset1 = dt_array[cnt];
			(*map)[i][j].lane_ctrl = dt_array[cnt + 1];
		}
		(*map_uc)[i].pp = &(*map)[i][0];
	}
	kfree(dt_array);
	return 0;

err_pdata_fail:
	kfree(dt_array);
err_port_map:
	return ret;
}

static int simple_amp_gpio_reset(struct simple_amp_priv *simple_amp, bool enable)
{
	int ret = 0;

	if (enable)
		ret = gpiod_direction_output(simple_amp->rst_gpio_np, 1);
	else
		ret = gpiod_direction_output(simple_amp->rst_gpio_np, 0);

	if (ret < 0) {
		dev_err_ratelimited(simple_amp->dev,
			"%s: failed to reset GPIO: %d\n", __func__, ret);
		return ret;
	}
	return ret;
}

static void simple_amp_regulator_disable(void *data)
{
	regulator_bulk_disable(SUPPLIES_NUM, data);
}

static int simple_amp_event_notify(struct notifier_block *nb,
				unsigned long val, void *ptr)
{
	u16 event = (val & 0xffff);
	struct simple_amp_priv *simple_amp = container_of(nb, struct simple_amp_priv,
						    parent_nblock);

	if (!simple_amp)
		return -EINVAL;

	switch (event) {
	case BOLERO_SLV_EVT_SSR_UP:
		break;

	default:
		dev_dbg(simple_amp->dev, "%s: unknown event %d\n",
			__func__, event);
		break;
	}

	return 0;
}

static int simple_amp_init(struct device *dev, struct regmap *regmap,
		struct swr_device *peripheral)
{
	struct simple_amp_priv *simple_amp = NULL;
	int ret, i;
	const char *simple_amp_codec_name_of = NULL;
	int dev_index = -1;
	u8 dev_num;
	struct  snd_soc_component_driver *component_drv = NULL;
	struct snd_soc_component *component = NULL;
	struct amp_ctrl_platform_data *plat_data =NULL;

	simple_amp = devm_kzalloc(dev, sizeof(*simple_amp), GFP_KERNEL);
	if (!simple_amp)
		return -ENOMEM;

	parser_sdca_data(dev, simple_amp);

	dev_set_drvdata(dev, simple_amp);
	simple_amp->swr_slave = peripheral;
	simple_amp->regmap = regmap;
	simple_amp->dev = dev;
	simple_amp->clk_freq = MCLK_9P6MHZ;

	for (i = 0; i < SUPPLIES_NUM; i++)
		simple_amp->supplies[i].supply = supply_name[i];

	ret = devm_regulator_bulk_get(dev, SUPPLIES_NUM,
			simple_amp->supplies);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get regulators\n");

	ret = regulator_bulk_enable(SUPPLIES_NUM, simple_amp->supplies);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to enable regulators\n");

	//Allocate resources
	ret = devm_add_action_or_reset(dev, simple_amp_regulator_disable,
			simple_amp->supplies);
	if (ret)
		return ret;

	simple_amp->rst_gpio_np = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(simple_amp->rst_gpio_np))
		return dev_err_probe(dev, PTR_ERR(simple_amp->rst_gpio_np),
				"Reset GPIO not found\n");

	ret = simple_amp_gpio_reset(simple_amp, true);
	if (ret != 0)
		return ret;
	/*
	 * Add 5msec delay to provide sufficient time for
	 * soundwire auto enumeration of slave devices as
	 * per HW requirement.
	 */
	usleep_range(5000, 5010);
	ret = swr_get_logical_dev_num(peripheral, peripheral->addr, &dev_num);
	if (ret) {
		dev_dbg(dev,
				"%s get dev_num %d for dev addr %llx failed\n",
				__func__, dev_num, peripheral->addr);
		ret = -EPROBE_DEFER;
		goto err;
	}
	peripheral->dev_num = dev_num;

	ret = of_property_read_string(dev->of_node, "qcom,codec-name", &simple_amp_codec_name_of);
	if (ret) {
		dev_err(dev, "Looking up %s property in node %s failed\n",
				"qcom,codec-name", dev->of_node->full_name);
		goto err;
	}

	component_drv = devm_kzalloc(dev, sizeof(*component_drv), GFP_KERNEL);
	if (!component_drv) {
		ret = -ENOMEM;
		goto err;
	}

	memcpy((void *)component_drv, &soc_codec_dev_simple_amp, sizeof(*component_drv));

	component_drv->name = devm_kstrdup(dev, simple_amp_codec_name_of, GFP_KERNEL);
	if (!component_drv->name) {
		ret = -ENOMEM;
		goto err;
	}
	simple_amp->driver = component_drv;

	ret = sscanf(simple_amp->driver->name, "sdca-simple-amp.%02d", &dev_index);
	if (ret != 1) {
		ret = -EINVAL;
		dev_err(dev, "name parsing for dev_index failed:%s\n", simple_amp->driver->name);
		goto err;
	}

	simple_amp->dai_driver = get_dai_driver(dev, dev_index);

	ret = devm_snd_soc_register_component(dev, simple_amp->driver,
			simple_amp->dai_driver, ARRAY_SIZE(simple_amp_dai));
	if (ret) {
		dev_err(dev, "Codec component %s registration failed\n",
				simple_amp->driver->name);
	} else {
		dev_dbg(dev, "Codec component %s registration success!\n",
				simple_amp->driver->name);
		dev_dbg(dev, "Codec component:dai %s %sregistration success!\n",
				simple_amp->dai_driver[0].name, simple_amp->dai_driver[1].name);
	}

	component = snd_soc_lookup_component(dev, simple_amp->driver->name);
	if (!component) {
		dev_err(dev, "%s: component is NULL\n", __func__);
		ret = -EINVAL;
	}

	simple_amp->parent_np = of_parse_phandle(dev->of_node,
			"qcom,lpass-cdc-handle", 0);
	if (simple_amp->parent_np) {
		simple_amp->parent_dev =
			of_find_device_by_node(simple_amp->parent_np);
		if (simple_amp->parent_dev) {
			plat_data = dev_get_platdata(&simple_amp->parent_dev->dev);
			if (plat_data) {
				simple_amp->parent_nblock.notifier_call =
					simple_amp_event_notify;
				if (plat_data->register_notifier)
					plat_data->register_notifier(
							plat_data->handle,
							&simple_amp->parent_nblock,
							true);
				simple_amp->register_notifier =
					plat_data->register_notifier;
				simple_amp->handle = plat_data->handle;
			} else {
				dev_err(dev, "%s: plat data not found\n",
						__func__);
			}
		} else {
			dev_err(dev, "%s: parent dev not found\n",
					__func__);
		}
	} else {
		dev_info(dev, "%s: parent node not found\n", __func__);
	}

	ret = parse_port_params(dev, "qcom,swr-amp-port-params");
	if (ret) {
		dev_err(dev, "Failed to read port params\n");
		goto err;
	}
	swr_init_port_params(simple_amp->swr_slave, SIMPLE_AMP_MAX_SWR_PORTS,
			simple_amp->swr_simple_amp_port_params);

#ifdef CONFIG_DEBUG_FS
	simple_amp_regdump_register(simple_amp, peripheral);
#endif

err:
	return ret;

}


void int_disable_handle(void) {

}

void pa0_ocp_int_handle(void) {

}

void pa1_ocp_int_handle(void) {

}

void clock_stop_int_handle(void) {

}

void uvlo_int_handle(void) {

}

void pa0_fsm_error_int_handle(void) {

}

void pa1_fsm_error_int_handle(void) {

}

void power_fsm_error_int_handle(void) {

}

void stereo_protection_Mode_Changed_handle(void) {

}

void stereo_playback_clock_valid_handle(void) {

}

void stereo_sense_clock_valid_handle(void) {

}

static int simple_amp_interrupt_cb(struct swr_device *swr_dev, u8 devnum)
{
	uint8_t stat1, stat2, stat3;
	struct simple_amp_priv *simple_amp = dev_get_drvdata(&swr_dev->dev);
	struct sdca_function *sdca_func_data = simple_amp->sdca_func_data[1];

	swr_read(swr_dev, devnum, SDW_SCP_SDCA_INT1 , &stat1, 1);
	swr_read(swr_dev, devnum, SDW_SCP_SDCA_INT2 , &stat2, 1);
	swr_read(swr_dev, devnum, SDW_SCP_SDCA_INT3 , &stat3, 1);

	switch (stat1 & (INT_DISABLE_MASK | PA0_OCP_INT_MASK | PA1_OCP_INT_MASK)) {
		case INT_DISABLE_MASK:
			int_disable_handle();
			break;
		case PA0_OCP_INT_MASK:
			pa0_ocp_int_handle();
			break;
		case PA1_OCP_INT_MASK:
			pa1_ocp_int_handle();
			break;
	}

	switch (stat2 & (PA0_FSM_ERROR_INT_HANDLE_MASK | PA1_FSM_ERROR_INT_HANDLE_MASK | POWER_FSM_ERROR_INT_HANDLE_MASK)) {
		case PA0_FSM_ERROR_INT_HANDLE_MASK:
			pa0_fsm_error_int_handle();
			break;
		case PA1_FSM_ERROR_INT_HANDLE_MASK:
			pa1_fsm_error_int_handle();
			break;
		case POWER_FSM_ERROR_INT_HANDLE_MASK:
			pa1_fsm_error_int_handle();
			break;
	}

	switch (stat3 & (STEREO_PROTECTION_MODE_CHANGED_HANDLE_MASK | STEREO_PLAYBACK_CLOCK_VALID_MASK | STEREO_SENSE_CLOCK_VALID_HANDLE_MASK)) {
		case STEREO_PROTECTION_MODE_CHANGED_HANDLE_MASK:
			stereo_protection_Mode_Changed_handle();
			break;
		case STEREO_PLAYBACK_CLOCK_VALID_MASK:
			stereo_playback_clock_valid_handle();
			break;
		case STEREO_SENSE_CLOCK_VALID_HANDLE_MASK:
			stereo_sense_clock_valid_handle();
			break;
	}
	pr_err("%s: entity Id: %d\n", __func__, sdca_func_data->entity_data[ENTITY_TYPE_CS_21].entity_id);

	return 0;
}

static int simple_amp_probe(struct swr_device *peripheral)
{
	struct regmap *regmap;

	peripheral->paging_support = true;

	/* Regmap Initialization */
	regmap = devm_regmap_init_swr(peripheral, &simple_amp_regmap);
	if (IS_ERR(regmap))
		return PTR_ERR(regmap);

	return simple_amp_init(&peripheral->dev, regmap, peripheral);
}

static int simple_amp_remove(struct swr_device *pdev)
{
	struct simple_amp_priv *simple_amp;

	simple_amp = swr_get_dev_data(pdev);
	if (!simple_amp) {
		dev_err(&pdev->dev, "%s: simple_amp is NULL\n", __func__);
		return -EINVAL;
	}

	if (simple_amp->register_notifier)
		simple_amp->register_notifier(simple_amp->handle,
				&simple_amp->parent_nblock, false);

#ifdef CONFIG_DEBUG_FS
	debugfs_remove_recursive(simple_amp->debugfs_dent);
	simple_amp->debugfs_dent = NULL;
#endif
	return 0;
}

static const struct dev_pm_ops simple_amp_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(simple_amp_suspend, simple_amp_resume)
};

static const struct of_device_id simple_amp_dt_match[] = {
	{
		.compatible = "qcom,simple-sdca-amp",
	},
	{}
};

static const struct swr_device_id simple_amp_id[] = {
	{"simple-sdca-amp", 0},
	{}
};

static struct swr_driver simple_amp_driver = {
	.driver = {
		.name = "simple-sdca-amp",
		.owner = THIS_MODULE,
		.pm = &simple_amp_pm_ops,
		.of_match_table = simple_amp_dt_match,
	},
	.probe = simple_amp_probe,
	.remove = simple_amp_remove,
	.interrupt_callback = simple_amp_interrupt_cb,
	.id_table = simple_amp_id,
};

static int __init simple_amp_swr_init(void)
{
	return swr_driver_register(&simple_amp_driver);
}

static void __exit simple_amp_swr_exit(void)
{
	swr_driver_unregister(&simple_amp_driver);
}

module_init(simple_amp_swr_init);
module_exit(simple_amp_swr_exit);

MODULE_DESCRIPTION("ASOC SDCA simple spkr Amp driver");
MODULE_LICENSE("GPL");
