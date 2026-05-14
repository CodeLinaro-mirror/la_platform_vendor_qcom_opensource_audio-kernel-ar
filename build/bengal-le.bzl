load(":audio_modules.bzl", "audio_modules")
load(":module_mgr.bzl", "define_target_modules")

def define_bengal_le():
    define_target_modules(
        target = "bengal-le",
        variants = ["consolidate", "perf", "debug-defconfig", "defconfig"],
        registry = audio_modules,
        modules = [
            # DSP & IPC Core
            "q6_dlkm",
            "spf_core_dlkm",
            "audpkt_ion_dlkm",
            "q6_notifier_dlkm",
            "adsp_loader_dlkm",
            "audio_prm_dlkm",
            "q6_pdr_dlkm",
            "gpr_dlkm",
            "audio_pkt_dlkm",
            # SoC Bus & Controllers
            "pinctrl_lpi_dlkm",
            "swr_dlkm",
            "swr_ctrl_dlkm",
            "snd_event_dlkm",
            # ASoC Machine Driver
            "machine_dlkm",
            # Codecs - Bengal primary
            "wcd_core_dlkm",
            "mbhc_dlkm",
            "wcd9xxx_dlkm",
            "stub_dlkm",
            "wcd937x_dlkm",
            "wcd937x_slave_dlkm",
            "wsa881x_analog_dlkm",
            "pm2250_spmi_dlkm",
            "rouleur_dlkm",
            "rouleur_slave_dlkm",
            # Bolero CDC Macros
            "bolero_cdc_dlkm",
            "va_macro_dlkm",
            "rx_macro_dlkm",
            "tx_macro_dlkm",
        ],
        config_options = [
            "CONFIG_SND_SOC_BENGAL",
            "CONFIG_SND_SOC_MSM_QDSP6V2_INTF",
            "CONFIG_SND_SOC_WCD_IRQ",
            "CONFIG_MSM_QDSP6_SSR",
            "CONFIG_MSM_CDC_PINCTRL",
            "CONFIG_SWRM_VER_1P7",
            "CONFIG_SOUNDWIRE_MSTR_CTRL",
            "CONFIG_WCD9XXX_CODEC_CORE_V2",
            "CONFIG_SND_SOC_WCD9XXX_V2",
            "CONFIG_DIGITAL_CDC_RSC_MGR",
            "CONFIG_SND_SOC_WCD_MBHC_ADC",
            "CONFIG_SND_SOC_ROULEUR",
            "CONFIG_SND_SOC_ROULEUR_SLAVE",
            "CONFIG_PM2250_SPMI",
        ],
    )
