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
#include <sound/tlv.h>
#include <linux/delay.h>
#include "asoc/bolero-slave-internal.h"
#include <bindings/audio-codec-port-types.h>
#include "simple-amp.h"

#define SIMPLE_AMP_RATES (SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
		SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000)
#define SIMPLE_AMP_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE | \
		SNDRV_PCM_FMTBIT_S32_LE)

#define SIMPLE_AMP_MAX_REGISTER 0x40C80800
#define MAX_CONTROL_SELECTORS  64
#define IS_VALID_FUNCTION(func)  (((func) > FUNCTION_ZERO) && ((func) < MAX_FUNCTION))

#ifdef CONFIG_DEBUG_FS
extern void simple_amp_regdump_register(struct simple_amp_priv *simple_amp,
	struct swr_device *pdev);
#endif

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
#define FU21_VOL_STEPS  124

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

static const DECLARE_TLV_DB_SCALE(fu21_digital_gain, -8400, 100, 0);

enum num_entity {
	ENTITY_TYPE_0,
	ENTITY_TYPE_IT_21,
	ENTITY_TYPE_CS_21,
	ENTITY_TYPE_PPU_21,
	ENTITY_TYPE_FU_21,
	ENTITY_TYPE_SAPU_29,
	ENTITY_TYPE_PDE_23,
	ENTITY_TYPE_OT_23,
	ENTITY_TYPE_IT_29,
	ENTITY_TYPE_CS_24,
	ENTITY_TYPE_OT_24,
	ENTITY_TYPE_TG_23,
	ENTITY_TYPE_MAX,
};

static const struct entity_idx_table entity_idx_table[] = {
	{.entity_id = ENTITY_TYPE_0,      .entity_label = "ENTITY0"},
	{.entity_id = ENTITY_TYPE_IT_21,  .entity_label = "IT21"},
	{.entity_id = ENTITY_TYPE_CS_21,  .entity_label = "CS21"},
	{.entity_id = ENTITY_TYPE_PPU_21, .entity_label = "PPU21"},
	{.entity_id = ENTITY_TYPE_FU_21,  .entity_label ="FU21"},
	{.entity_id = ENTITY_TYPE_SAPU_29,.entity_label = "SAPU21"},
	{.entity_id = ENTITY_TYPE_PDE_23, .entity_label = "PDE23"},
	{.entity_id = ENTITY_TYPE_OT_23,  .entity_label = "OT23"},
	{.entity_id = ENTITY_TYPE_IT_29,  .entity_label = "IT29"},
	{.entity_id = ENTITY_TYPE_CS_24,  .entity_label = "CS24"},
	{.entity_id = ENTITY_TYPE_OT_24,  .entity_label = "OT24"},
	{.entity_id = ENTITY_TYPE_TG_23,  .entity_label = "TG23"},
};

enum access_mode {
	READ_ONLY = 0,
	WRITE_ONLY = 1,
	READ_WRITE = 2,
	INVALID_MODE,
};

enum mode_table {
	MODE_0 = 0,
	MODE_1,
	MODE_2,
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

struct amp_ctrl_platform_data {
	void *handle;
	int (*update_amp_event)(void *handle, u16 event, u32 data);
	int (*register_notifier)(void *handle, struct notifier_block *nblock,
				bool enable);
};

struct sdca_control_sel_data {
	u32 access_layer;
	enum access_mode access_mode;
};

struct sdca_entity {
	u32 entity_num;
	const char *entity_label;
	u32 *cntrl_nums; /* Control numbers */
	u32 num_cntrl_len; /* size for Control numbers */
	struct sdca_control_sel_data *control_sel;
	int num_ctl_sel;
	u32 *control_sel_list; /* Array for control selector values */
	struct sdca_control_sel_data **control_sel_map;
};

struct sdca_function {
	struct sdca_entity entity_data[ENTITY_TYPE_MAX];
	u32 *init_table;
	int init_table_size;
};

static int get_id_by_label(const char *label)
{
	int i;
	size_t label_len = strlen(label);

	for (i = 0; i < ARRAY_SIZE(entity_idx_table); i++) {
		if (strncmp(entity_idx_table[i].entity_label,
						label, label_len) == 0) {
				return entity_idx_table[i].entity_id;
		}
	}
	return -1;
}

static unsigned int get_access_mode(struct sdca_entity *entity_data,
			unsigned int ctl_sel)
{
	int i = 0;

	if (!entity_data || !entity_data->control_sel_map)
		return INVALID_MODE;

	for (i = 0; i < entity_data->num_ctl_sel; i++) {
		if (entity_data->control_sel_list[i] == ctl_sel)
			return entity_data->control_sel_map[ctl_sel]->access_mode;
	}
	return INVALID_MODE;
}

static bool simple_amp_readable_register(struct device *dev, unsigned int reg)
{
	unsigned int fun, ent, ctl_sel, ch;
	unsigned int access_mode;
	struct sdca_entity *entity_data;
	struct sdca_function *sdca_fun = NULL;
	struct simple_amp_priv *simple_amp = dev_get_drvdata(dev);

	if (!simple_amp)
		return false;

	SDW_SDCA_EXTRACT_ALL(reg, fun, ent, ctl_sel, ch);

	if (!IS_VALID_FUNCTION(fun))
		return false;

	sdca_fun = simple_amp->sdca_func_data[fun];
	if (!sdca_fun)
		return false;

	entity_data = &sdca_fun->entity_data[ent];

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
	struct sdca_function *sdca_fun = NULL;
	struct simple_amp_priv *simple_amp = dev_get_drvdata(dev);

	if (!simple_amp)
		return false;

	SDW_SDCA_EXTRACT_ALL(reg, fun, ent, ctl_sel, ch);

	if (!IS_VALID_FUNCTION(fun))
		return false;

	sdca_fun = simple_amp->sdca_func_data[fun];
	if (!sdca_fun)
		return false;

	entity_data = &sdca_fun->entity_data[ent];

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
	struct sdca_function *sdca_fun = NULL;

	struct simple_amp_priv *simple_amp = dev_get_drvdata(dev);
	if (!simple_amp)
		return false;

	SDW_SDCA_EXTRACT_ALL(reg, fun, ent, ctl_sel, ch);

	if (!IS_VALID_FUNCTION(fun))
		return false;

	sdca_fun = simple_amp->sdca_func_data[fun];
	if (!sdca_fun)
		return false;

	entity_data = &sdca_fun->entity_data[ent];

	access_mode = get_access_mode(entity_data, ctl_sel);

	if (access_mode == READ_ONLY)
		return true;
	else
		return false;
}

static struct regmap_config simple_amp_regmap = {
	.reg_bits = 32,
	.val_bits = 8,
	.readable_reg = simple_amp_readable_register,
	.volatile_reg = simple_amp_volatile_register,
	.writeable_reg = simple_amp_writeable_register,
	.max_register = SIMPLE_AMP_MAX_REGISTER,
	.cache_type = REGCACHE_MAPLE,
	.use_single_read = true,
	.use_single_write = true,
};


static int parse_control_selectors(struct device *dev, struct sdca_entity *entity,
		int ctl_sel_idx, struct device_node *subproperty_np)
{
	int ret = 0;
	u32 control_selector;
	u32 access_mode;
	struct sdca_control_sel_data *control_sel = NULL;

	control_selector = entity->control_sel_list[ctl_sel_idx];
	if (control_selector >= MAX_CONTROL_SELECTORS) {
		dev_err(dev, "%s: cnt_sel %d > Max cnt sel %d\n",
		__func__, control_selector, MAX_CONTROL_SELECTORS);
		return -EINVAL;
	}

	control_sel = devm_kzalloc(dev,
			sizeof(struct sdca_control_sel_data), GFP_KERNEL);
	if (!control_sel) {
		dev_err(dev,
		"%s: failed to allocate memory for cntl selectors values\n",
			__func__);
		return -ENOMEM;
	}

	ret = of_property_read_u32(subproperty_np,
			"mipi-sdca-control-access-layer", &control_sel->access_layer);
	if (ret) {
		dev_err(dev,
		"%s: failed to read access layer for selector 0x%x subproperty, ret: %d\n",
				__func__, control_selector, ret);
		return ret;
	}

	ret = of_property_read_u32(subproperty_np, "mipi-sdca-control-access-mode",
			&access_mode);
	if (ret) {
		dev_err(dev,
		"%s: failed to read access mode for selector 0x%x subproperty\n",
				__func__, control_selector);
		return ret;
	}

	switch (access_mode) {
		case 0:
			control_sel->access_mode = READ_ONLY;
			break;
		case 1:
			control_sel->access_mode = WRITE_ONLY;
			break;
		case 2:
			control_sel->access_mode = READ_WRITE;
			break;
		default:
			dev_err(dev, "%s: Unknown access mode %d\n", __func__,
					access_mode);
			return -EINVAL;
	}

	entity->control_sel_map[control_selector] = control_sel;

	return 0;
}

static int parse_entity(struct device *dev, struct device_node *function_node,
		struct sdca_function *func_data)
{
	struct sdca_entity *entity_data;
	struct device_node *entity_np, *control_np;
	struct device_node *sub_np;
	int ret, i, idx;
	const char *entity_label;
	char property_name[50];

	for_each_child_of_node(function_node, entity_np) {

		/* Read Entity Label */
		if (of_property_read_string(entity_np, "mipi-sdca-entity-label",
					&entity_label)) {
			dev_err(dev, "%s: Failed to read entity label\n",
				__func__);
			return -EINVAL;
		}

		/* Get Entity Idx by label */
		idx = get_id_by_label(entity_label);
		if (idx < 0) {
			dev_err(dev, "%s: entity label %s not found\n",
						 __func__, entity_label);
			return -EINVAL;
		}
		entity_data = &func_data->entity_data[idx];

		ret = of_property_read_u32(entity_np, "mipi-sdca-entity-number",
				&entity_data->entity_num);
		if (ret) {
			dev_err(dev, "%s: Failed to read entity number\n",
					__func__);
			return -EINVAL;
		}

		entity_data->num_cntrl_len = of_property_count_u32_elems(entity_np,
			"mipi-sdca-control-numbers");
		if (entity_data->num_cntrl_len < 0) {
			dev_err(dev, "%s: Failed to get len of control numbers\n",
				__func__);
			return entity_data->num_cntrl_len;
		}

		entity_data->cntrl_nums = devm_kzalloc(dev,
			entity_data->num_cntrl_len * sizeof(u32), GFP_KERNEL);
		if (!entity_data->cntrl_nums) {
			dev_err(dev,
				"%s: Failed to allocate memory for cntrl_nums\n",
				__func__);
			return -ENOMEM;
		}

		ret = of_property_read_u32_array(entity_np,
			"mipi-sdca-control-numbers", entity_data->cntrl_nums,
				entity_data->num_cntrl_len);
		if (ret) {
			dev_err(dev,
			"%s: Failed to read control number: %d ent_num = %d\n",
					__func__, ret, entity_data->entity_num);
			return ret;
		}

		/* Count No of Selector numbers */
		entity_data->num_ctl_sel = of_property_count_elems_of_size(entity_np,
				"mipi-sdca-control-selectors", sizeof(u32));
		if (entity_data->num_ctl_sel < 0) {
			dev_err(dev,
				"%s: failed to count control selector elements\n",
					__func__);
			return entity_data->num_ctl_sel;
		}
		/* Create Memory for Control sel Array list */
		entity_data->control_sel_list = devm_kzalloc(dev,
				entity_data->num_ctl_sel * sizeof(u32), GFP_KERNEL);
		if (!entity_data->control_sel_list) {
			dev_err(dev,
				"%s: Failed to allocate memory for cntl_sels\n",
					__func__);
			return -ENOMEM;
		}

		/* Parse and store the control selector number in the Array list */
		ret = of_property_read_u32_array(entity_np, "mipi-sdca-control-selectors",
				entity_data->control_sel_list, entity_data->num_ctl_sel);
		if (ret < 0) {
			dev_err(dev, "%s: Failed to read %s\n", __func__,
					"mipi-sdca-control-selectors");
			return ret;
		}

		/* Parse the sub-node sdca_control_list */
		control_np = of_get_child_by_name(entity_np, "control-selectors");
		if (!control_np) {
			dev_err(dev,
			"%s: Failed to find control list node\n", __func__);
			return -EINVAL;
		}

		entity_data->control_sel_map = devm_kzalloc(dev,
			MAX_CONTROL_SELECTORS * sizeof(struct sdca_control_sel_data *),
			GFP_KERNEL);
		if (!entity_data->control_sel_map) {
			dev_err(dev,
			"%s: Failed to allocate memory for cntl_sel_map, ret: %d\n",
			__func__, ret);
			return -ENOMEM;
		}

		for (i = 0; i < entity_data->num_ctl_sel; i++) {
			snprintf(property_name, sizeof(property_name),
					"mipi-sdca-control-0x%X-subproperties",
					entity_data->control_sel_list[i]);
			sub_np = of_parse_phandle(control_np, property_name, 0);
			if (!sub_np) {
				dev_err(dev,
				"%s: Invalid phandle property: %s, entity_num: %d\n",
				__func__, property_name, entity_data->entity_num);
				return -EINVAL;
			}

			ret = parse_control_selectors(dev, entity_data, i, sub_np);
			if (ret) {
				dev_err(dev,
					"%s: Failed to parse mipi-sdca-control-0x%X-subproperties ret: %d\n",
					__func__, entity_data->control_sel_list[i], ret);
				return ret;
			}
		}
	}

	return 0;
}

static int parse_reg_defaults(struct device *dev,
					struct regmap_config *simple_amp_regmap)
{
	struct reg_default *temp_defaults;
	const __be32 *prop;
	int len, i;

	prop = of_get_property(dev->of_node, "qcom,reg_defaults", &len);
	if (!prop)
		return -EINVAL;

	len /= sizeof(u32);

	/* Allocate memory for reg_defaults */
	temp_defaults = devm_kzalloc(dev, len * sizeof(struct reg_default),
						GFP_KERNEL);
	if (!temp_defaults)
		return -ENOMEM;

	for (i = 0; i < len; i += 2) {
		temp_defaults[i / 2].reg = be32_to_cpu(prop[i]);
		temp_defaults[i / 2].def = be32_to_cpu(prop[i + 1]);
	}

	simple_amp_regmap->reg_defaults = temp_defaults;
	simple_amp_regmap->num_reg_defaults = len / 2;

	return 0;
}

static int parse_initialization_table(struct device *dev,
		struct device_node *function_node, struct sdca_function *sdca_func)
{
	sdca_func->init_table_size = of_property_count_u32_elems(function_node,
			"mipi-sdca-function-initialization-table");
	if (sdca_func->init_table_size < 0) {
		dev_err(dev, "%s: Failed to count elements from init table\n",
				__func__);
		return sdca_func->init_table_size;
	}

	if (sdca_func->init_table_size % 2 != 0) {
		dev_err(dev, "%s: Invalid number of elements in init table\n",
				__func__);
		return -EINVAL;
	}

	sdca_func->init_table = devm_kzalloc(dev,
			sdca_func->init_table_size * sizeof(u32), GFP_KERNEL);
	if (!sdca_func->init_table)
		return -ENOMEM;

	if (of_property_read_u32_array(function_node,
				"mipi-sdca-function-initialization-table",
				sdca_func->init_table,
				sdca_func->init_table_size)) {
		dev_err(dev,
		"%s: Failed to read mipi-sdca-function-initialization-table\n",
				__func__);
		return -EINVAL;
	}

	return 0;
}

static int parse_functions(struct device *dev, struct device_node *function_node,
		struct sdca_function *sdca_func_data)
{
	int ret = 0;

	if (!sdca_func_data)
		return -EINVAL;

	ret = parse_initialization_table(dev, function_node, sdca_func_data);
	if (ret) {
		dev_err(dev, "init table parse failed\n");
		return ret;
	}

	ret = parse_entity(dev, function_node, sdca_func_data);
	if (ret)
		dev_err(dev,
		"%s: Failed to parse Entities ret: %d\n", __func__, ret);

	return ret;
}

static int parser_sdca_data(struct device *dev, struct simple_amp_priv *simple_amp)
{
	int i, ret = 0;
	u32 functions[MAX_FUNCTION];
	struct device_node *np = dev->of_node;
	struct device_node *function_node;

	simple_amp->num_functions = of_property_count_elems_of_size(dev->of_node,
			"qcom,simple-amp-function-array", sizeof(u32));
	if (simple_amp->num_functions < 0 || simple_amp->num_functions > MAX_FUNCTION) {
		dev_err(dev, "invalid num functions:%d\n", simple_amp->num_functions);
		return -EINVAL;
	}

	if (of_property_read_u32_array(dev->of_node,
				"qcom,simple-amp-function-array",
				functions, simple_amp->num_functions)) {
		dev_err(dev, "%s: Failed to read function array\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < simple_amp->num_functions; ++i) {
		if (IS_VALID_FUNCTION(functions[i])) {
			simple_amp->sdca_func_data[functions[i]] = devm_kzalloc(dev,
					sizeof(struct sdca_function), GFP_KERNEL);
			if (!simple_amp->sdca_func_data[functions[i]])
				return -ENOMEM;
		}
	}

	i = 0;
	for_each_child_of_node(np, function_node) {
		struct sdca_function *sdca_func_data =
					simple_amp->sdca_func_data[functions[i]];
		ret = parse_functions(dev, function_node, sdca_func_data);
		if (ret) {
			dev_err(dev, "parse function %d failed\n", functions[i]);
			break;
		}
		i++;
	}

	return ret;
}

static const struct snd_soc_dapm_route simple_amp_audio_map[] = {
	{"OUT", NULL, "IN"},
};

static const struct snd_soc_dapm_widget simple_amp_dapm_widgets[] = {
	SND_SOC_DAPM_INPUT("IN"),
	SND_SOC_DAPM_OUTPUT("OUT"),
};

/*
 * The input string is duplicated to avoid modifying the original string. The
 * string is split into tokens,and each token is parsed to extract reg-val pair.
 */
static int parse_and_store_registers(struct simple_amp_priv *simple_amp,
		const char *input)
{
	char *str, *token, *tofree;
	uint32_t reg, value;
	size_t count = 0;

	/* duplicates the input string and allocates memory for it using the
	   kernels memory allocator.*/
	tofree = str = kstrdup(input, GFP_KERNEL);
	if (!str)
		return -ENOMEM;

	/* First pass: count the number of register-value pairs
	 * splits the string str into tokens based on the delimiter " ".
	 * token points to the current token, and str is updated to point to the
	 * remainder of the string. converts the token to a 32-bit unsigned
	 * integer (reg). The second parameter 0 indicates that the function
	 * should automatically detect the base (hexadecimal, decimal, etc.).
	 */
	while ((token = strsep(&str, " ")) != NULL) {
		if (kstrtou32(token, 0, &reg) == 0) {
			token = strsep(&str, " ");
			if (token && kstrtou32(token, 0, &value) == 0) {
				count++;
			} else {
				pr_err("%s: Invalid value for register 0x%x\n",
						__func__, reg);
				kfree(tofree);
				return -EINVAL;
			}
		} else {
			pr_err("%s: Invalid register format: %s\n",
					__func__, token);
			kfree(tofree);
			return -EINVAL;
		}
	}

	kfree(tofree);

	if (count >= NUM_IMP_DEF_REG_VAL_PAIRS)
		return -EINVAL;

	simple_amp->num_pairs = count;
	tofree = str = kstrdup(input, GFP_KERNEL);
	if (!str)
		return -ENOMEM;

	count = 0;
	while ((token = strsep(&str, " ")) != NULL) {
		if (kstrtou32(token, 0, &reg) == 0) {
			token = strsep(&str, " ");
			if (token && kstrtou32(token, 0, &value) == 0) {
				simple_amp->reg_val_pair[count].reg = reg;
				simple_amp->reg_val_pair[count].value = value;
				count++;
			}
		}
	}

	kfree(tofree);
	return 0;
}

/* Put and Get for Implementation registers */
static int simple_amp_impl_def_reg_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	return 0;
}

static int simple_amp_impl_def_reg_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp =
				snd_soc_component_get_drvdata(component);
	const char *input = ucontrol->value.bytes.data;

	dev_dbg(component->dev, "%s:Impl defined reg val pair: %s\n", __func__,
			input);
	return parse_and_store_registers(simple_amp, input);
}

static const char *dummy_enum_text[] = { "Dummy" };

static int simple_amp_stereo_vol_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp =
				snd_soc_component_get_drvdata(component);

	ucontrol->value.integer.value[0] = simple_amp->stereo_voldB;

	return 0;
}

static int simple_amp_stereo_vol_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp =
				snd_soc_component_get_drvdata(component);

	simple_amp->stereo_voldB = ucontrol->value.integer.value[0];

	return 0;
}

/* Get and Put the usage Modes */
static int simple_amp_usage_modes_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp =
				snd_soc_component_get_drvdata(component);

	if (!simple_amp)
		return -EINVAL;

	ucontrol->value.integer.value[0] = simple_amp->usage_mode;

	return 0;
}

static int simple_amp_usage_modes_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp =
				snd_soc_component_get_drvdata(component);

	if (!simple_amp)
		return -EINVAL;

	simple_amp->usage_mode = ucontrol->value.integer.value[0];

	dev_dbg(component->dev, "%s: Usage mode:%d\n", __func__,
			simple_amp->usage_mode);

	return 0;
}

/* Get and Put for RX Select Channel */
static int simple_amp_rx_ch_sel_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp =
				snd_soc_component_get_drvdata(component);

	ucontrol->value.enumerated.item[0] = simple_amp->rx_ch_sel;

	return 0;
}

static int simple_amp_rx_ch_sel_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp =
			snd_soc_component_get_drvdata(component);

	simple_amp->rx_ch_sel = ucontrol->value.enumerated.item[0];

	dev_dbg(component->dev, "%s: Rx channel:%d select\n", __func__,
			simple_amp->rx_ch_sel);
	return 0;
}

static const char * const simple_amp_rx_ch_sel_enum_text[] = {
	"CH_NONE", "CH1", "CH2", "CH_Stereo",
};

/* get and put for Debug mode */
static int simple_amp_debug_mode_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp =
				snd_soc_component_get_drvdata(component);

	ucontrol->value.integer.value[0] = simple_amp->debug_mode_enable;
	return 0;
}

static int simple_amp_debug_mode_put(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct simple_amp_priv *simple_amp =
				snd_soc_component_get_drvdata(component);

	simple_amp->debug_mode_enable = ucontrol->value.enumerated.item[0];

	dev_dbg(component->dev, "%s: Debug Mode Enable: %d\n", __func__,
			simple_amp->debug_mode_enable);
	return 0;
}

static const char *const debug_mode_enum[] = {"Disable", "Enable"};

static const struct snd_kcontrol_new simple_amp_snd_controls[] = {
	SOC_SINGLE_EXT("OT23 Usage Mode", SND_SOC_NOPM, 0, 8, 0,
			simple_amp_usage_modes_get, simple_amp_usage_modes_put),

	SOC_SINGLE_EXT_TLV("FU21 Stereo Volume DB", SND_SOC_NOPM,
			0, FU21_VOL_STEPS, 0, simple_amp_stereo_vol_get,
			simple_amp_stereo_vol_put, fu21_digital_gain),

	SOC_ENUM_EXT("Rx Channel select", simple_amp_rx_ch_sel_enum_text,
			simple_amp_rx_ch_sel_get, simple_amp_rx_ch_sel_put),

	SOC_ENUM_EXT("Debug Mode", debug_mode_enum, simple_amp_debug_mode_get,
			simple_amp_debug_mode_put),

	SOC_ENUM_EXT("Impl def Reg val", dummy_enum_text,
		simple_amp_impl_def_reg_get, simple_amp_impl_def_reg_put),
};

static void simple_amp_function_init(struct simple_amp_priv *simple_amp,
		struct sdca_function *sdca_func)
{
	int i;

	for (i = 0; i < sdca_func->init_table_size; i += 2) {
		regmap_write(simple_amp->regmap, sdca_func->init_table[i],
				sdca_func->init_table[i + 1]);
	}
}

static int write_mode_table(struct regmap * regmap, int num_regs,
		const u32 *reg_values)
{
	int i;

	if (num_regs < 0) {
		pr_err("%s: No registers to write\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < num_regs; i += 2)
		regmap_write(regmap, reg_values[i], reg_values[i + 1]);

	return 0;
}

static int simple_amp_component_probe(struct snd_soc_component *component)
{
	int num_regs, ret, i;
	u32 reg_values[MAX_INIT_REGS * 2];
	struct simple_amp_priv *simple_amp =
				snd_soc_component_get_drvdata(component);
	struct sdca_function *sdca_func_data = NULL;
	struct sdca_entity *entity_data = NULL;
	simple_amp = snd_soc_component_get_drvdata(component);

	if (!simple_amp)
		return -EINVAL;

	simple_amp->component = component;
	snd_soc_component_init_regmap(component, simple_amp->regmap);

	devm_regmap_qti_debugfs_register(simple_amp->dev, simple_amp->regmap);

	for (i = FUNCTION_ZERO; i < MAX_FUNCTION; ++i) {
		struct sdca_function *sdca_func_data =
						simple_amp->sdca_func_data[i];
		if (sdca_func_data)
			simple_amp_function_init(simple_amp, sdca_func_data);
	}

	/* Update mode_0 lookup table */
	num_regs = of_property_read_variable_u32_array(simple_amp->dev->of_node,
			"mode_0_tbl", reg_values, 0, ARRAY_SIZE(reg_values));
	if (num_regs == -EOVERFLOW) {
		dev_err(simple_amp->dev,
			"num of DT entries exceed expected for mode_0_table\n");
		return -EINVAL;
	}
	if (num_regs == -ENODATA || num_regs == -EINVAL) {
		num_regs = 0; /* mode_0_tbl is optional */
		dev_dbg(simple_amp->dev, "mode 0 table not found\n");
	}

	ret = write_mode_table(simple_amp->regmap, num_regs, reg_values);
	if (ret < 0)
		return -EINVAL;

	/* Update mode_1 lookup table */
	num_regs = of_property_read_variable_u32_array(simple_amp->dev->of_node,
			"mode_1_tbl", reg_values, 0, ARRAY_SIZE(reg_values));
	if (num_regs == -EOVERFLOW) {
		dev_err(simple_amp->dev,
			"num of DT entries exceed expected for mode_1_table\n");
		return -EINVAL;
	}
	if (num_regs == -ENODATA || num_regs == -EINVAL) {
		num_regs = 0; /* mode_1_tbl is optional */
		dev_dbg(simple_amp->dev, "mode 1 table not found\n");
	}

	ret = write_mode_table(simple_amp->regmap, num_regs, reg_values);
	if (ret < 0)
		return -EINVAL;

	/* Update mode_2 lookup table */
	num_regs = of_property_read_variable_u32_array(simple_amp->dev->of_node,
			"mode_2_tbl", reg_values, 0, ARRAY_SIZE(reg_values));
	if (num_regs == -EOVERFLOW) {
		dev_err(simple_amp->dev,
			"num of DT entries exceed expected for mode_2_table\n");
		return -EINVAL;
	}
	if (num_regs == -ENODATA || num_regs == -EINVAL) {
		num_regs = 0; /* mode_2_tbl is optional */
		dev_dbg(simple_amp->dev, "mode 2 table not found\n");
	}

	ret = write_mode_table(simple_amp->regmap, num_regs, reg_values);
	if (ret < 0)
		return -EINVAL;

	/* Set the posture number, one time setting for platform
	   ----------------------------------------------|
	   |Posture_num   | RX0            |RX1          |
	   |--------------|----------------|-------------|
	   |0x1           | PORT_1- CH1    |  PORT_1- CH2|
	   |------------- |----------------|-------------|
	   |0x2           | PORT_1- CH2    |  PORT_1- CH1|
	   ------------------------------------------------------|
	 */

	for (i = FUNCTION_ZERO; i < MAX_FUNCTION; ++i) {
		sdca_func_data = simple_amp->sdca_func_data[i];
		if (!sdca_func_data)
			continue;

		entity_data = &sdca_func_data->entity_data[ENTITY_TYPE_PPU_21];
		regmap_write(simple_amp->regmap,
				SDW_SDCA_CTL(i, entity_data->entity_num,
					SIMPLE_AMP_CTL_POSTURE_NUM, 0),
				0x1);
	}

	return 0;
}

static void simple_amp_component_remove(struct snd_soc_component *component)
{
	struct simple_amp_priv *simple_amp =
			snd_soc_component_get_drvdata(component);

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
	int i;
	struct simple_amp_priv *simple_amp = snd_soc_dai_get_drvdata(dai);

	if (!simple_amp)
		return -EINVAL;

	if (simple_amp->debug_mode_enable) {
		for (i = 0; i < simple_amp->num_pairs; i++) {
			regmap_write(simple_amp->regmap,
					simple_amp->reg_val_pair[i].reg,
					simple_amp->reg_val_pair[i].value);
		}
	}

	return 0;
}

#define ENTITY_FU21  0x4
#define SIMPLE_AMP_FU21_CH_VOL_CH2X0 SDW_SDCA_CTL(FUNCTION_STEREO,\
		ENTITY_FU21, SIMPLE_AMP_CTL_FU21_VOLUME, CH1)
#define SIMPLE_AMP_FU21_CH_VOL_CH2X1 SDW_SDCA_CTL(FUNCTION_STEREO,\
		ENTITY_FU21, SIMPLE_AMP_CTL_FU21_VOLUME, CH2)

#define SIMPLE_AMP_FU21_LEFT_CH_VOL_CH2X0 SDW_SDCA_CTL(FUNCTION_MONO_CH0,\
		ENTITY_FU21, SIMPLE_AMP_CTL_FU21_VOLUME, CH1)
#define SIMPLE_AMP_FU21_RIGHT_CH_VOL_CH2X1 SDW_SDCA_CTL(FUNCTION_MONO_CH1,\
		ENTITY_FU21, SIMPLE_AMP_CTL_FU21_VOLUME, CH1)


#define SIMPLE_AMP_CN_MBQ_ENABLE_MASK      (0x6000)
#define SIMPLE_AMP_CN_ENABLE_MASK          (0x4000)

#define SIMPLE_AMP_FU21_CH_VOL_CH2X0_MSB   (SIMPLE_AMP_FU21_CH_VOL_CH2X0 | SIMPLE_AMP_CN_MBQ_ENABLE_MASK)
#define SIMPLE_AMP_FU21_CH_VOL_CH2X0_LSB   (SIMPLE_AMP_FU21_CH_VOL_CH2X0 | SIMPLE_AMP_CN_ENABLE_MASK)
#define SIMPLE_AMP_FU21_CH_VOL_CH2X1_MSB   (SIMPLE_AMP_FU21_CH_VOL_CH2X1 | SIMPLE_AMP_CN_MBQ_ENABLE_MASK)
#define SIMPLE_AMP_FU21_CH_VOL_CH2X1_LSB   (SIMPLE_AMP_FU21_CH_VOL_CH2X1 | SIMPLE_AMP_CN_ENABLE_MASK)

static void simple_amp_set_volume(struct simple_amp_priv *simple_amp, int func)
{
	int msb_val = 0, lsb_val = 0;

	switch (func) {
		case FUNCTION_STEREO:
			/* Update Vol for CH1 */
			msb_val = simple_amp->stereo_voldB;
			regmap_write(simple_amp->regmap,
				SIMPLE_AMP_FU21_CH_VOL_CH2X0_MSB, msb_val);
			regmap_write(simple_amp->regmap,
				SIMPLE_AMP_FU21_CH_VOL_CH2X0_LSB, lsb_val);

			/* Update Vol for CH2 */
			regmap_write(simple_amp->regmap,
				SIMPLE_AMP_FU21_CH_VOL_CH2X1_MSB, msb_val);
			regmap_write(simple_amp->regmap,
				SIMPLE_AMP_FU21_CH_VOL_CH2X1_LSB, lsb_val);
			break;
		case FUNCTION_MONO_CH0:
			msb_val = simple_amp->ch1_voldB;
			/* Update Vol for LEFT_CH2X0 - 0x40800211 */
			regmap_write(simple_amp->regmap,
				SIMPLE_AMP_FU21_LEFT_CH_VOL_CH2X0 | SIMPLE_AMP_CN_MBQ_ENABLE_MASK,
				msb_val);
			regmap_write(simple_amp->regmap,
				SIMPLE_AMP_FU21_LEFT_CH_VOL_CH2X0 | SIMPLE_AMP_CN_ENABLE_MASK,
				lsb_val);
			break;
		case FUNCTION_MONO_CH1:
			msb_val = simple_amp->ch1_voldB;
			/* Update Vol for RIGHT_CH2X1 - 0x40C00211 */
			regmap_write(simple_amp->regmap,
				SIMPLE_AMP_FU21_RIGHT_CH_VOL_CH2X1 | SIMPLE_AMP_CN_MBQ_ENABLE_MASK,
				msb_val);
			regmap_write(simple_amp->regmap,
				SIMPLE_AMP_FU21_RIGHT_CH_VOL_CH2X1 | SIMPLE_AMP_CN_ENABLE_MASK,
				lsb_val);
			break;
		default:
			dev_dbg(simple_amp->dev, "%s: invalid function: %d\n",
					__func__, func);
	}
}

static int wait_for_pde_state(struct simple_amp_priv *simple_amp,
		int pde23_entity_num, int cs21_entity_num, int ps, int func_num)
{
	int act_ps, cnt = 0, clock_valid;
	int rc = 0;

	do {
		usleep_range(1000, 1500);
		/* wait and read actual_PS */
		rc = regmap_read(simple_amp->regmap,
				SDW_SDCA_CTL(func_num, pde23_entity_num,
					SIMPLE_AMP_CTL_PDE_ACT_PS, 0),
				&act_ps);

		if (rc == 0 && act_ps == ps)
			return rc;
	} while (++cnt < 5);

	regmap_read(simple_amp->regmap,
			SDW_SDCA_CTL(func_num, cs21_entity_num,
				SIMPLE_AMP_CTL_CLOCK_VALID, 0),
			&clock_valid);
	dev_err(simple_amp->dev,
		"simple amp ps%d request failed, func num %d act_ps %d, cs_valid:%d\n",
			ps, func_num, act_ps, clock_valid);
	return -EINVAL;

}

static void get_ch_mask(u8 ch_mask[], int num_channels, int channel_sel)
{
	if (num_channels == 1) {
		if (channel_sel == CH1) {
			ch_mask[0] = 0x1;
			ch_mask[1] = 0x1;
			ch_mask[2] = 0x4;
		} else if (channel_sel == CH2) {
			ch_mask[0] = 0x2;
			ch_mask[1] = 0x2;
			ch_mask[2] = 0x8;
		}
	} else {
		ch_mask[0] = 0x3;
		ch_mask[1] = 0x3;
		ch_mask[2] = 0xc;
	}
}

static void prepare_channel(struct port_config *config, int func,
		int num_channels, int channel_sel)
{
	u8 ch1 = 0x1, ch2 = 0x2;
	u8 ch1_2 = ch1 | ch2;

	if (func == FUNCTION_STEREO) {
		if (num_channels == 1) {
			if (channel_sel == CH1) {
				swr_write(config->swr_slave,
					config->swr_slave->dev_num,
					SWRS_DP_PREPARE_CONTROL(PORT_1), &ch1);
			} else if (channel_sel == CH2) {
				swr_write(config->swr_slave,
					config->swr_slave->dev_num,
					SWRS_DP_PREPARE_CONTROL(PORT_1), &ch2);
			}
		} else {
			swr_write(config->swr_slave, config->swr_slave->dev_num,
				SWRS_DP_PREPARE_CONTROL(PORT_1), &ch1);
			swr_write(config->swr_slave, config->swr_slave->dev_num,
				SWRS_DP_PREPARE_CONTROL(PORT_1), &ch1_2);
		}
		/* delay 2ms for Channels to become ready */
		/* TODO: poll for CH ready instead */
		usleep_range(2000, 2010);
	} else if (func == FUNCTION_MONO_CH0) {
		swr_write(config->swr_slave, config->swr_slave->dev_num,
			SWRS_DP_PREPARE_CONTROL(PORT_1), &ch1);
		usleep_range(2000, 2010);
	} else if (func == FUNCTION_MONO_CH1) {
		swr_write(config->swr_slave, config->swr_slave->dev_num,
			SWRS_DP_PREPARE_CONTROL(PORT_2), &ch2);
		usleep_range(2000, 2010);
	}
}

static void simple_amp_get_port_param(struct port_config *config,
		int func, int num_channels, unsigned int rate,
		int channel_sel)
{

	if (!config || !config->swr_slave) {
		pr_err("%s: Invalid input parameters\n", __func__);
		return;
	}

	memset(config, 0, sizeof(*config));

	switch (func) {
		case FUNCTION_STEREO:
			{
				u8 port_id[] = {PORT_1, PORT_3, PORT_3};
				u8 port_type[] = {OCPM, SPKR_IPCM, CPS};
				u8 num_ch[] = {num_channels, num_channels, num_channels};
				unsigned int ch_rate[] = {rate, rate, rate};

				memcpy(config->port_id, port_id, sizeof(port_id));
				memcpy(config->port_type, port_type, sizeof(port_type));
				memcpy(config->num_ch, num_ch, sizeof(num_ch));
				memcpy(config->ch_rate, ch_rate, sizeof(ch_rate));
				get_ch_mask(config->ch_mask, num_channels, channel_sel);
				config->num_ports = 3;
			}
			break;
		case FUNCTION_MONO_CH0:
			{
				u8 port_id[] = {PORT_1, PORT_3};
				u8 port_type[] = {OCPM, SPKR_IPCM};
				u8 num_ch[] = {num_channels, num_channels};
				unsigned int ch_rate[] = {rate, rate};
				u8 ch_mask[] = {0x1, 0x3};

				memcpy(config->port_id, port_id, sizeof(port_id));
				memcpy(config->port_type, port_type, sizeof(port_type));
				memcpy(config->num_ch, num_ch, sizeof(num_ch));
				memcpy(config->ch_rate, ch_rate, sizeof(ch_rate));
				memcpy(config->ch_mask, ch_mask, sizeof(ch_mask));
				config->num_ports = 2;
			}
			break;
		case FUNCTION_MONO_CH1:
			{
				u8 port_id[] = {PORT_2, PORT_4};
				u8 port_type[] = {OCPM, CPS};
				u8 num_ch[] = {num_channels, num_channels};
				unsigned int ch_rate[] = {48000, 48000};
				u8 ch_mask[] = {0x1, 0x3};

				memcpy(config->port_id, port_id, sizeof(port_id));
				memcpy(config->port_type, port_type, sizeof(port_type));
				memcpy(config->num_ch, num_ch, sizeof(num_ch));
				memcpy(config->ch_rate, ch_rate, sizeof(ch_rate));
				memcpy(config->ch_mask, ch_mask, sizeof(ch_mask));
				config->num_ports = 2;
			}
			break;
		default:
			pr_err("%s: Invalid function\n", __func__);
			return;
	}

	prepare_channel(config, func, num_channels, channel_sel);
}


static int simple_amp_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	int ret = 0;
	u8 commit_val = 0x02;
	unsigned char ps0 = 0x0;
	struct snd_soc_component *component = dai->component;
	struct simple_amp_priv *simple_amp =
		snd_soc_component_get_drvdata(component);
	struct sdca_function *sdca_func_data =
				simple_amp->sdca_func_data[dai->id];
	struct sdca_entity *entity_data = NULL;
	struct port_config *pconfig_sel = NULL;
	int num_channels = 0, channel_sel = GENMASK(1, 0);
	unsigned int rx_sample_rate, tx_sample_rate;

	if (!simple_amp || !simple_amp->swr_slave)
		return -EINVAL;

	if(dai->id == FUNCTION_STEREO)
		pconfig_sel = &simple_amp->pconfig.pconfig_stereo;
	else if(dai->id == FUNCTION_MONO_CH0)
		pconfig_sel = &simple_amp->pconfig.dual_mono.pconfig_mono_ch1;
	else if(dai->id == FUNCTION_MONO_CH1)
		pconfig_sel = &simple_amp->pconfig.dual_mono.pconfig_mono_ch2;
	else {
		dev_err(component->dev, "%s: Invalid Dai ID: %d\n",
				__func__, dai->id);
		return -EINVAL;
	}

	memset(pconfig_sel, 0, sizeof(struct port_config));

	num_channels = params_channels(params);

	switch (params_rate(params)) {
		case 8000:
			rx_sample_rate = SIMPLE_AMP_RX_RATE_8000HZ;
			tx_sample_rate = SIMPLE_AMP_VI_RATE_8000HZ;
			break;
		case 16000:
			rx_sample_rate = SIMPLE_AMP_RX_RATE_16000HZ;
			tx_sample_rate = SIMPLE_AMP_VI_RATE_16000HZ;
			break;
		case 32000:
			rx_sample_rate = SIMPLE_AMP_RX_RATE_32000HZ;
			break;
		case 44100:
			rx_sample_rate = SIMPLE_AMP_RX_RATE_44100HZ;
			tx_sample_rate = SIMPLE_AMP_VI_RATE_44100HZ;
			break;
		case 48000:
			rx_sample_rate = SIMPLE_AMP_RX_RATE_48000HZ;
			tx_sample_rate = SIMPLE_AMP_VI_RATE_48000HZ;
			break;
		case 96000:
			rx_sample_rate = SIMPLE_AMP_RX_RATE_96000HZ;
			tx_sample_rate = SIMPLE_AMP_VI_RATE_96000HZ;
			break;
		case 192000:
			rx_sample_rate = SIMPLE_AMP_RX_RATE_192000HZ;
			tx_sample_rate = SIMPLE_AMP_VI_RATE_192000HZ;
			break;
		case 22050:
			tx_sample_rate = SIMPLE_AMP_VI_RATE_22050HZ;
			break;
		case 24000:
			tx_sample_rate = SIMPLE_AMP_VI_RATE_24000HZ;
			break;
		default:
			dev_err(component->dev, "Rate %d is not supported\n",
					params_rate(params));
			ret = -EINVAL;
			goto exit;
	}

	dev_dbg(simple_amp->dev, "%s(): dai_name = %s, stream = %s channels: %d, rate: %u",
			__func__, dai->name, substream->name, num_channels,
			params_rate(params));

	/* Limit bit-width to 16 for now. 24/32 to be added */
	if (params_width(params) != 16) {
		dev_err(simple_amp->dev, "unsupported bit width %d\n",
				params_width(params));
		return -EINVAL;
	}

	/* configure RX sample rate */
	entity_data = &sdca_func_data->entity_data[ENTITY_TYPE_CS_21];
	regmap_write(simple_amp->regmap,
			SDW_SDCA_CTL(dai->id, entity_data->entity_num,
				SIMPLE_AMP_CTL_SAMPLE_FREQ_INDEX, 0),
			rx_sample_rate);

	/* configure VI sample rate */
	entity_data = &sdca_func_data->entity_data[ENTITY_TYPE_CS_24];
	regmap_write(simple_amp->regmap,
			SDW_SDCA_CTL(dai->id, entity_data->entity_num,
				SIMPLE_AMP_CTL_SAMPLE_FREQ_INDEX, 0),
			tx_sample_rate);

	/* configure usage mode */
	entity_data = &sdca_func_data->entity_data[ENTITY_TYPE_OT_23];
	regmap_write(simple_amp->regmap,
			SDW_SDCA_CTL(dai->id, entity_data->entity_num,
			SIMPLE_AMP_CTL_IT_USAGE, 0), simple_amp->usage_mode);

	/* configure cluster Index */
	entity_data = &sdca_func_data->entity_data[ENTITY_TYPE_IT_21];
	regmap_write(simple_amp->regmap,
			SDW_SDCA_CTL(dai->id, entity_data->entity_num,
				SIMPLE_AMP_CTL_CLUSTER_INDEX, 0), 0x1);

	/* configure Volume */
	simple_amp_set_volume(simple_amp, dai->id);

	swr_write(simple_amp->swr_slave, simple_amp->swr_slave->dev_num,
			SWRS_SCP_COMMIT, &commit_val);

	/* Set PDE23 control */
	struct sdca_entity *pde23_entity =
		&sdca_func_data->entity_data[ENTITY_TYPE_PDE_23];
	regmap_write(simple_amp->regmap,
			SDW_SDCA_CTL(dai->id, pde23_entity->entity_num,
				SIMPLE_AMP_PDE_REQ_PS, 0), ps0);

	struct sdca_entity *cs21_entity =
		&sdca_func_data->entity_data[ENTITY_TYPE_CS_21];
	ret = wait_for_pde_state(simple_amp,
			pde23_entity->entity_num,
			cs21_entity->entity_num, ps0, dai->id);
	if (!ret)
		dev_dbg(component->dev, "success! function %d, actual ps %d",
				dai->id, ps0);

	pconfig_sel->swr_slave = simple_amp->swr_slave;

	if (num_channels == 1)
		channel_sel = simple_amp->rx_ch_sel;

	simple_amp_get_port_param(pconfig_sel, dai->id, num_channels,
			params_rate(params), channel_sel);

	snd_soc_dai_dma_data_set_playback(dai, pconfig_sel);

	ret = swr_connect_port(simple_amp->swr_slave, pconfig_sel->port_id,
			pconfig_sel->num_ports, pconfig_sel->ch_mask, pconfig_sel->ch_rate,
			pconfig_sel->num_ch, pconfig_sel->port_type);
	if (ret != 0) {
		dev_err(component->dev, "%s: swr connect port failed: %d\n",
				__func__, ret);
		goto exit;
	}

	ret = swr_slvdev_datapath_control(simple_amp->swr_slave,
			simple_amp->swr_slave->dev_num, true);
	if (ret != 0) {
		dev_err(component->dev, "%s: slvdev datapath control error: %d\n",
				__func__, ret);
		goto exit;
	}

exit:
	return ret;
}

static int simple_amp_prepare(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	u8 commit_val = 0x02;
	struct snd_soc_component *component = dai->component;
	struct simple_amp_priv *simple_amp =
				snd_soc_component_get_drvdata(component);
	struct sdca_function *sdca_func = simple_amp->sdca_func_data[dai->id];
	struct sdca_entity *fu21_entity =
				&sdca_func->entity_data[ENTITY_TYPE_FU_21];

	if (!simple_amp || !sdca_func || !fu21_entity)
		return -EINVAL;

	/* Unmute the 0x40400209 - Channel 0 and 0x4040020A - channel 1 */
	regmap_write(simple_amp->regmap,
		(SDW_SDCA_CTL(dai->id, fu21_entity->entity_num,
			SIMPLE_AMP_CTL_FU21_MUTE,
			fu21_entity->cntrl_nums[CH1]) | BIT(14)),
			0x00);
	regmap_write(simple_amp->regmap,
			(SDW_SDCA_CTL(dai->id, fu21_entity->entity_num,
			SIMPLE_AMP_CTL_FU21_MUTE,
			fu21_entity->cntrl_nums[CH2]) | BIT(14)),
			0x00);

	swr_write(simple_amp->swr_slave, simple_amp->swr_slave->dev_num,
			SWRS_SCP_COMMIT, &commit_val);

	return 0;
}

static void simple_amp_shutdown(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{

}

static int simple_amp_hw_free(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	int ret = 0;
	struct port_config *pconfig_sel = NULL;
	struct snd_soc_component *component = dai->component;
	struct simple_amp_priv *simple_amp =
			snd_soc_component_get_drvdata(component);

	pconfig_sel = snd_soc_dai_dma_data_get_playback(dai);

	ret = swr_disconnect_port(simple_amp->swr_slave, pconfig_sel->port_id,
			pconfig_sel->num_ports, pconfig_sel->ch_mask,
			pconfig_sel->port_type);
	if (ret != 0)
		dev_err(component->dev, "%s: swr disconnect port failed: %d\n",
				__func__, ret);

	ret = swr_slvdev_datapath_control(simple_amp->swr_slave,
				simple_amp->swr_slave->dev_num, false);
	if (ret != 0)
		dev_err(component->dev, "%s: swr slv dath control failed: %d\n",
				__func__, ret);

	return ret;
}

static const struct snd_soc_dai_ops simple_amp_dai_ops = {
	.startup = simple_amp_startup,
	.hw_params = simple_amp_hw_params,
	.prepare = simple_amp_prepare,
	.shutdown = simple_amp_shutdown,
	.hw_free = simple_amp_hw_free,
};


static struct snd_soc_dai_driver simple_amp_dai[] = {
	{
		.name = "",
		.id = FUNCTION_STEREO,
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
		.id = FUNCTION_MONO_CH0,
		.playback = {
			.stream_name = "",
			.rates = SIMPLE_AMP_RATES,
			.formats = SIMPLE_AMP_FORMATS,
			.rate_max = 192000,
			.rate_min = 8000,
			.channels_min = 1,
			.channels_max = 1,
		},
		.ops = &simple_amp_dai_ops,
	},
	{
		.name = "",
		.id = FUNCTION_MONO_CH1,
		.playback = {
			.stream_name = "",
			.rates = SIMPLE_AMP_RATES,
			.formats = SIMPLE_AMP_FORMATS,
			.rate_max = 192000,
			.rate_min = 8000,
			.channels_min = 1,
			.channels_max = 1,
		},
		.ops = &simple_amp_dai_ops,
	},
};


static struct snd_soc_dai_driver *get_dai_driver(struct device *dev,
				int dev_index)
{
	struct snd_soc_dai_driver *dai_drv = NULL;

	dai_drv =  devm_kzalloc(dev,
			ARRAY_SIZE(simple_amp_dai) * sizeof(struct snd_soc_dai_driver),
			GFP_KERNEL);
	if (!dai_drv)
		return NULL;


	memcpy(dai_drv, simple_amp_dai,
			ARRAY_SIZE(simple_amp_dai) * sizeof(struct snd_soc_dai_driver));

	dai_drv[0].name = devm_kasprintf(dev, GFP_KERNEL, "simple_amp_stereo_%d",
			dev_index);
	dai_drv[1].name = devm_kasprintf(dev, GFP_KERNEL, "simple_amp_mono_ch0_%d",
			dev_index);
	dai_drv[2].name = devm_kasprintf(dev, GFP_KERNEL, "simple_amp_mono_ch1_%d",
			dev_index);
	if (!dai_drv[0].name || !dai_drv[1].name || !dai_drv[2].name)
		return NULL;

	dai_drv[0].playback.stream_name = devm_kasprintf(dev, GFP_KERNEL,
			"Simple Amp AIF%d Stereo Playback", dev_index);
	dai_drv[1].playback.stream_name = devm_kasprintf(dev, GFP_KERNEL,
			"Simple Amp AIF%d Mono_Ch0 Playback", dev_index);
	dai_drv[2].playback.stream_name = devm_kasprintf(dev, GFP_KERNEL,
			"Simple Amp AIF%d Mono_Ch1 Playback", dev_index);
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
		dev_err_ratelimited(dev, "%s: simple_amp private data is NULL\n",
				__func__);
		return -EINVAL;
	}
	dev_dbg(dev, "%s: system resume\n", __func__);
	return 0;
}

static int simple_amp_gpio_set(struct simple_amp_priv *simple_amp, bool val)
{
	int ret = 0;

	if (val)
		ret = gpiod_direction_output(simple_amp->sd_n, 1);
	else
		ret = gpiod_direction_output(simple_amp->sd_n, 0);

	if (ret < 0) {
		dev_err_ratelimited(simple_amp->dev,
				"%s: failed to set GPIO: %d\n", __func__, ret);
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
	struct amp_ctrl_platform_data *plat_data =NULL;

	simple_amp = devm_kzalloc(dev, sizeof(*simple_amp), GFP_KERNEL);
	if (!simple_amp)
		return -ENOMEM;

	ret = parser_sdca_data(dev, simple_amp);
	if (ret) {
		dev_err(dev, "sdca simple amp DT parse failed\n");
		return ret;
	}

	dev_set_drvdata(dev, simple_amp);
	simple_amp->swr_slave = peripheral;
	simple_amp->regmap = regmap;
	simple_amp->dev = dev;
	simple_amp->clk_freq = MCLK_9P6MHZ;
	simple_amp->stereo_voldB = -84; /* default state */

	for (i = 0; i < SUPPLIES_NUM; i++)
		simple_amp->supplies[i].supply = supply_name[i];

	ret = devm_regulator_bulk_get(dev, SUPPLIES_NUM, simple_amp->supplies);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get regulators\n");

	ret = regulator_bulk_enable(SUPPLIES_NUM, simple_amp->supplies);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to enable regulators\n");

	ret = devm_add_action_or_reset(dev, simple_amp_regulator_disable,
			simple_amp->supplies);
	if (ret) {
		dev_err(dev, "failed to devm_add_action_or_reset, %d\n", ret);
		return ret;
	}

	simple_amp->sd_n = devm_gpiod_get_optional(dev, "powerdown",
							GPIOD_OUT_HIGH);
	if (IS_ERR(simple_amp->sd_n))
		return dev_err_probe(dev, PTR_ERR(simple_amp->sd_n),
				"Shutdown Control GPIO not found\n");

	ret = simple_amp_gpio_set(simple_amp, false);
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

	ret = of_property_read_string(dev->of_node, "qcom,codec-name",
			&simple_amp_codec_name_of);
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

	memcpy((void *)component_drv, &soc_codec_dev_simple_amp,
			sizeof(*component_drv));

	component_drv->name = devm_kstrdup(dev, simple_amp_codec_name_of, GFP_KERNEL);
	if (!component_drv->name) {
		ret = -ENOMEM;
		goto err;
	}
	simple_amp->driver = component_drv;

	ret = sscanf(simple_amp->driver->name, "sdca-simple-amp.%02d", &dev_index);
	if (ret != 1) {
		ret = -EINVAL;
		dev_err(dev, "name parsing for dev_index failed:%s\n",
				simple_amp->driver->name);
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
		dev_dbg(dev, "Codec component:dai %s %s %s registration success!\n",
			simple_amp->dai_driver[0].name, simple_amp->dai_driver[1].name,
			simple_amp->dai_driver[2].name);
	}

	simple_amp->parent_np = of_parse_phandle(dev->of_node,
			"qcom,lpass-cdc-handle", 0);
	if (simple_amp->parent_np) {
		simple_amp->parent_dev = of_find_device_by_node(simple_amp->parent_np);
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

#ifdef CONFIG_DEBUG_FS
	simple_amp_regdump_register(simple_amp, peripheral);
#endif

err:
	return ret;

}


void int_disable_handle(void)
{
}

void pa0_ocp_int_handle(void)
{
}

void pa1_ocp_int_handle(void)
{
}

void clock_stop_int_handle(void)
{
}

void uvlo_int_handle(void)
{
}

void pa0_fsm_error_int_handle(void)
{
}

void pa1_fsm_error_int_handle(void)
{
}

void power_fsm_error_int_handle(void)
{
}

void stereo_protection_Mode_Changed_handle(void)
{
}

void stereo_playback_clock_valid_handle(void)
{
}

void stereo_sense_clock_valid_handle(void)
{
}

static int simple_amp_interrupt_cb(struct swr_device *swr_dev, u8 devnum)
{
	uint32_t stat1, stat2, stat3;
	uint32_t tmp;
	struct simple_amp_priv *simple_amp = dev_get_drvdata(&swr_dev->dev);

	dev_dbg(simple_amp->dev, "%s: Enter\n", __func__);

	do {

		swr_read(swr_dev, devnum, SDW_SCP_SDCA_INT1 , &stat1, 1);
		swr_read(swr_dev, devnum, SDW_SCP_SDCA_INT2 , &stat2, 1);
		swr_read(swr_dev, devnum, SDW_SCP_SDCA_INT3 , &stat3, 1);

		// Check and handle each bit for register1
		if (stat1 & SDW_SCP_SDCA_INTMASK_SDCA_3) {
			int_disable_handle();
		}
		if (stat1 & SDW_SCP_SDCA_INTMASK_SDCA_4) {
			pa0_ocp_int_handle();
		}
		if (stat1 & SDW_SCP_SDCA_INTMASK_SDCA_5) {
			pa1_ocp_int_handle();
		}

		// Check and handle each bit for register2
		if (stat2 & SDW_SCP_SDCA_INTMASK_SDCA_8) {
			clock_stop_int_handle();
		}
		if (stat2 & SDW_SCP_SDCA_INTMASK_SDCA_11) {
			uvlo_int_handle();
		}
		if (stat2 & SDW_SCP_SDCA_INTMASK_SDCA_13) {
			pa0_fsm_error_int_handle();
		}
		if (stat2 & SDW_SCP_SDCA_INTMASK_SDCA_14) {
			pa1_fsm_error_int_handle();
		}
		if (stat2 & SDW_SCP_SDCA_INTMASK_SDCA_15) {
			power_fsm_error_int_handle();
		}

		// Check and handle each bit for register3
		if (stat3 & SDW_SCP_SDCA_INTMASK_SDCA_21) {
			stereo_protection_Mode_Changed_handle();
		}
		if (stat3 & SDW_SCP_SDCA_INTMASK_SDCA_22) {
			stereo_playback_clock_valid_handle();
		}
		if (stat3 & SDW_SCP_SDCA_INTMASK_SDCA_23) {
			stereo_sense_clock_valid_handle();
		}

		tmp =(uint32_t) ~SDCA_INT1_MASK;
		stat1 = stat1 & tmp;

		tmp =(uint32_t) ~SDCA_INT2_MASK;
		stat2 = stat2 & tmp;

		tmp =(uint32_t) ~SDCA_INT3_MASK;
		stat3 = stat3 & tmp;

		// Clear the interrupt( write-back 0 to clear the interrupt)
		swr_write(swr_dev, devnum, SDW_SCP_SDCA_INT1, &stat1);
		swr_write(swr_dev, devnum, SDW_SCP_SDCA_INT2, &stat2);
		swr_write(swr_dev, devnum, SDW_SCP_SDCA_INT3, &stat3);

		// Read the register values again to check for new interrupts
		swr_read(swr_dev, devnum, SDW_SCP_SDCA_INT1 , &stat1, 1);
		swr_read(swr_dev, devnum, SDW_SCP_SDCA_INT2 , &stat2, 1);
		swr_read(swr_dev, devnum, SDW_SCP_SDCA_INT3 , &stat3, 1);

	} while ((stat1 & SDCA_INT1_MASK) ||
			(stat2 & SDCA_INT2_MASK) ||
			(stat3 & SDCA_INT3_MASK)); // Continue if any of the bits are still set

	return 0;
}

static int simple_amp_probe(struct swr_device *peripheral)
{
	struct regmap *regmap;

	peripheral->paging_support = true;

	parse_reg_defaults(&peripheral->dev, &simple_amp_regmap);

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

MODULE_DESCRIPTION("ASoC SDCA simple speaker amp driver");
MODULE_LICENSE("GPL");
