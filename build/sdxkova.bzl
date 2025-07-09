load(":audio_modules.bzl", "audio_modules")
load(":module_mgr.bzl", "define_target_modules")
load("//build/kernel/kleaf:kernel.bzl", "ddk_module")
load("//msm-kernel:target_variants.bzl", "get_all_variants", "targets")
load("//build/bazel_common_rules/dist:dist.bzl", "copy_to_dist_dir")

def define_sdxkova():
    define_target_modules(
        target = "sdxkova",
        variants = ["debug-defconfig", "perf-defconfig"],
        registry = audio_modules,
        modules = [
            "q6_dlkm", # dsp/Kbuild
            "spf_core_dlkm",
            "audpkt_ion_dlkm",
            "q6_notifier_dlkm",
            "adsp_loader_dlkm",
            "audio_prm_dlkm",
            "q6_pdr_dlkm",
            "gpr_dlkm", # ipc/
            "audio_pkt_dlkm",
            "machine_dlkm", # asoc/
            "platform_dlkm",
            "snd_event_dlkm", #soc/
            "swr_dlkm",
            "swr_ctrl_dlkm",
            "pinctrl_wcd_dlkm",
            "stub_dlkm", # asoc/codecs
            "mbhc_dlkm",
            "wcd_core_dlkm",
            "wcd9xxx_dlkm",
            "wcd934x_dlkm",
            "wsa881x_dlkm",
        ],
        config_options = [
            "CONFIG_PINCTRL_WCD", #pinctrl
            "CONFIG_MSM_CDC_PINCTRL",
            "CONFIG_SND_SOC_WCD9XXX_V2", # asoc/codecs
            "CONFIG_SND_SOC_WCD934X",
            "CONFIG_SND_SOC_WCD934X_MBHC",
            "CONFIG_SND_SOC_WCD934X_DSD",
            "CONFIG_WCD9XXX_CODEC_CORE",
            "CONFIG_SND_SOC_WSA881X",
            "CONFIG_SND_SOC_WCD_MBHC",
            "CONFIG_SND_SOC_WCD_MBHC_ADC",
            "CONFIG_SND_SOC_MSM_STUB",
            "CONFIG_AUDIO_EXT_CLK",
            "CONFIG_SND_SOC_SDX", # asoc/
            "CONFIG_SND_SOC_QDSP6V2",
            "CONFIG_SND_EVENT", # soc/
            "CONFIG_SOUNDWIRE",
            "CONFIG_SOUNDWIRE_WCD_CTRL",
            "CONFIG_REGMAP_SWR",
            "CONFIG_SPF_CORE", # dsp/
            "CONFIG_AUDIO_PRM",
            "CONFIG_MDM_AUDIO_SSR",
            "CONFIG_MSM_QDSP6_SSR",
            "CONFIG_MSM_QDSP6_PDR",
            "CONFIG_MSM_QDSP6_NOTIFIER",
            "CONFIG_MSM_ADSP_LOADER",
            "CONFIG_SND_SOC_MSM_QDSP6V2_INTF",
            "CONFIG_MSM_QDSP6_GPR_RPMSG", # gpr/
            "CONFIG_AUDIO_PKT", #ipc/
            "CONFIG_AUDIO_PKT_ION",
            "CONFIG_WCD934X_I2S",  # misc/
            "CONFIG_AUDIO_EXT_CLK",
            "CONFIG_AUDIO_GPR_DOMAIN_MODEM",
            "CONFIG_SND_SOC_MSM_HOSTLESS_PCM",
            "CONFIG_MSM_GLINK_SPI_XPRT",
            "CONFIG_WCD_DSP_GLINK",
        ]
    )
