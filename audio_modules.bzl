load(":module_mgr.bzl", "create_module_registry")

DSP_PATH = "dsp"
IPC_PATH = "ipc"
SOC_PATH = "soc"
ASOC_PATH = "asoc"
ASOC_CODECS_PATH = ASOC_PATH + "/codecs"
ASOC_CODECS_BOLERO_PATH = ASOC_CODECS_PATH + "/bolero"

audio_modules = create_module_registry([":audio_headers"])
# ------------------------------------ AUDIO MODULE DEFINITIONS ---------------------------------
# >>>> DSP MODULES <<<<
audio_modules.register(
    name = "spf_core_dlkm",
    path = DSP_PATH,
    config_option = "CONFIG_SPF_CORE",
    srcs = ["spf-core.c"],
    conditional_srcs = {
        "CONFIG_DIGITAL_CDC_RSC_MGR": [
            "digital-cdc-rsc-mgr.c"
        ]
    },
    deps = [":%b_gpr_dlkm"],
)

audio_modules.register(
    name = "audpkt_ion_dlkm",
    path = DSP_PATH,
    config_option = "CONFIG_AUDIO_PKT_ION",
    srcs = ["msm_audio_ion.c"]
)
audio_modules.register(
    name = "q6_notifier_dlkm",
    path = DSP_PATH,
    config_option = "CONFIG_MSM_QDSP6_NOTIFIER",
    srcs = [
        "audio_notifier.c",
        "audio_ssr.c"
    ],
    deps = [":%b_q6_pdr_dlkm"],
)

audio_modules.register(
    name = "q6_dlkm",
    path = DSP_PATH,
    conditional_srcs = {
        "CONFIG_SND_SOC_MSM_QDSP6V2_INTF": [
            "msm-audio-event-notify.c",
            "q6_init.c",
        ],
        "CONFIG_SND_SOC_MSM_QDSP6V2_VM": [
            "msm-audio-event-notify.c",
            "msm_audio_ion_vm.c",
            "q6_init.c",
        ],
        "CONFIG_MSM_AVTIMER": [
            "avtimer.c"
        ],
        "CONFIG_XT_LOGGING": [
            "sp_params.c"
        ]
    }
)

audio_modules.register(
    name = "adsp_loader_dlkm",
    path = DSP_PATH,
    config_option = "CONFIG_MSM_ADSP_LOADER",
    srcs = ["adsp-loader.c"],
    deps = [":%b_spf_core_dlkm"],
)
audio_modules.register(
    name = "audio_prm_dlkm",
    path = DSP_PATH,
    config_option = "CONFIG_AUDIO_PRM",
    srcs = ["audio_prm.c"],
    deps = [":%b_spf_core_dlkm",
            ":%b_gpr_dlkm",
            ":%b_q6_notifier_dlkm",
	   ],
)

audio_modules.register(
    name = "q6_pdr_dlkm",
    path = DSP_PATH,
    config_option = "CONFIG_MSM_QDSP6_PDR",
    srcs = ["audio_pdr.c"]
)

# >>>> IPC MODULES <<<<
audio_modules.register(
    name = "gpr_dlkm",
    path = IPC_PATH,
    config_option = "CONFIG_MSM_QDSP6_GPR_RPMSG",
    srcs = ["gpr-lite.c"],
    deps = [":%b_q6_notifier_dlkm",
            ":%b_snd_event_dlkm",
	   ],
)
audio_modules.register(
    name = "audio_pkt_dlkm",
    path = IPC_PATH,
    config_option = "CONFIG_AUDIO_PKT",
    srcs = ["audio-pkt.c"],
    deps = [":%b_spf_core_dlkm",
            ":%b_gpr_dlkm",
            ":%b_audpkt_ion_dlkm",
	   ],
)
# >>>> SOC MODULES <<<<
audio_modules.register(
    name = "snd_event_dlkm",
    path = SOC_PATH,
    config_option = "CONFIG_SND_EVENT",
    srcs = ["snd_event.c"]
)
audio_modules.register(
    name = "pinctrl_lpi_dlkm",
    path = SOC_PATH,
    config_option = "CONFIG_PINCTRL_LPI",
    srcs = ["pinctrl-lpi.c"],
    deps = [":%b_spf_core_dlkm",
            ":%b_q6_notifier_dlkm",
            ":%b_snd_event_dlkm",
	   ],
)
audio_modules.register(
    name = "swr_dlkm",
    path = SOC_PATH,
    config_option = "CONFIG_SOUNDWIRE",
    srcs = [
        "regmap-swr.c",
        "soundwire.c"
    ]
)
audio_modules.register(
    name = "swr_ctrl_dlkm",
    path = SOC_PATH,
    conditional_srcs = {
        "CONFIG_SOUNDWIRE_WCD_CTRL": [
            "swr-wcd-ctrl.c"
        ],
        "CONFIG_SOUNDWIRE_MSTR_CTRL": [
            "swr-mstr-ctrl.c"
        ]
    },
    deps = [":%b_spf_core_dlkm",
            ":%b_q6_notifier_dlkm",
            ":%b_snd_event_dlkm",
            ":%b_swr_dlkm",
	   ],
)

# >>>> ASOC/CODEC MODULES <<<<
audio_modules.register(
    name = "wcd_core_dlkm",
    path = ASOC_CODECS_PATH,
    conditional_srcs = {
        "CONFIG_WCD9XXX_CODEC_CORE": [
            "wcd9xxx-rst.c",
            "wcd9xxx-core-init.c",
            "wcd9xxx-core.c",
            "wcd9xxx-irq.c",
            "wcd9xxx-slimslave.c",
            "wcd9xxx-utils.c",
            "wcd9335-regmap.c",
            "wcd9335-tables.c",
            "msm-cdc-pinctrl.c",
            "msm-cdc-supply.c",
            "wcd934x/wcd934x-regmap.c",
            "wcd934x/wcd934x-tables.c",
        ],
        "CONFIG_WCD9XXX_CODEC_CORE_V2": [
            "wcd9xxx-core-init.c",
            "msm-cdc-pinctrl.c",
            "msm-cdc-supply.c",
        ],
        "CONFIG_SND_SOC_WCD_IRQ": [
            "wcd-irq.c"
        ]
    }
)
audio_modules.register(
    name = "mbhc_dlkm",
    path = ASOC_CODECS_PATH,
    config_option = "CONFIG_SND_SOC_WCD_MBHC",
    srcs = ["wcd-mbhc-v2.c"],
    deps = [":%b_swr_dlkm"],
    conditional_srcs = {
        "CONFIG_SND_SOC_WCD_MBHC_ADC": [
            "wcd-mbhc-adc.c"
        ],
        "CONFIG_SND_SOC_WCD_MBHC_LEGACY": [
            "wcd-mbhc-legacy.c"
        ]
    },
)

audio_modules.register(
    name = "wcd9xxx_dlkm",
    path = ASOC_CODECS_PATH,
    config_option = "CONFIG_SND_SOC_WCD9XXX_V2",
    srcs = [
        "wcdcal-hwdep.c",
        "wcd9xxx-soc-init.c",
        "audio-ext-clk-up.c"
    ],
    conditional_srcs = {
        "CONFIG_WCD9XXX_CODEC_CORE": {
            True: [
                "wcd9xxx-common-v2.c",
                "wcd9xxx-resmgr-v2.c",
                "wcd-dsp-utils.c",
                "wcd-dsp-mgr.c",
            ],
            False: [
                "wcd-clsh.c"
            ]
        }
    },
    deps = [":%b_audio_prm_dlkm"],
)
audio_modules.register(
    name = "swr_haptics_dlkm",
    path = ASOC_CODECS_PATH,
    config_option = "CONFIG_SND_SWR_HAPTICS",
    srcs = ["swr-haptics.c"],
    deps = [":%b_swr_dlkm"],
)

audio_modules.register(
    name = "stub_dlkm",
    path = ASOC_CODECS_PATH,
    config_option = "CONFIG_SND_SOC_MSM_STUB",
    srcs = ["msm_stub.c"]
)

# >>>> ASOC MODULES <<<<
audio_modules.register(
    name = "machine_dlkm",
    path = ASOC_PATH,
    srcs = [
        "msm_common.c",
    ],
    conditional_srcs = {
        "CONFIG_SND_SOC_SM8150": [
            "sm8150.c",
            "machine_815x_init.c"
        ],
        "CONFIG_SND_SOC_SM6150": [
            "sm6150.c",
            "machine_615x_init.c"
        ],
        "CONFIG_SND_SOC_SA6155": [
            "sa6155.c"
        ],
        "CONFIG_SND_SOC_QCS405": [
            "qcs405.c"
        ],
        "CONFIG_SND_SOC_KONA": [
            "kona.c"
        ],
        "CONFIG_SND_SOC_WAIPIO": [
            "waipio.c"
        ],
        "CONFIG_SND_SOC_KALAMA": [
            "kalama.c"
        ],
        "CONFIG_SND_SOC_PINEAPPLE": [
            "pineapple.c"
        ],
        "CONFIG_SND_SOC_HOLI": [
            "holi.c"
        ],
        "CONFIG_SND_SOC_LITO": [
            "kona.c"
        ],
        "CONFIG_SND_SOC_BENGAL": [
            "bengal.c"
        ],
        "CONFIG_SND_SOC_SA8155": [
            "sa8155.c"
        ],
        "CONFIG_SND_SOC_SDX": [
            "sdx-target.c"
        ],
        "CONFIG_SND_SOC_TUNA": [
            "sun.c"
        ],
        "CONFIG_SND_SOC_SUN": [
            "sun.c"
        ],
	"CONFIG_SND_SOC_VIENNA": [
            "vienna.c"
        ],
        "CONFIG_SND_SOC_CANOE": [
            "audio_machine.c"
        ]
    },

    deps = [":%b_spf_core_dlkm",
            ":%b_audio_prm_dlkm",
            ":%b_bolero_cdc_dlkm",
            ":%b_wcd_core_dlkm",
            ":%b_rx_macro_dlkm",
            ":%b_wcd937x_dlkm",
            ":%b_wsa881x_analog_dlkm",
            ":%b_snd_event_dlkm",
            ":%b_rouleur_dlkm",
           ],
)

# >>>> ASOC/CODECS/BOLERO MODULES <<<<
audio_modules.register(
    name = "bolero_cdc_dlkm",
    path = ASOC_CODECS_BOLERO_PATH,
    config_option = "CONFIG_SND_SOC_BOLERO",
    srcs = [
        "bolero-cdc.c",
        "bolero-cdc-utils.c",
        "bolero-cdc-regmap.c",
        "bolero-cdc-tables.c",
        "bolero-clk-rsc.c",
    ],
     deps = [":%b_spf_core_dlkm",
            ":%b_snd_event_dlkm",
           ],
)
audio_modules.register(
    name = "va_macro_dlkm",
    path = ASOC_CODECS_BOLERO_PATH,
    config_option = "CONFIG_VA_MACRO",
    srcs = ["va-macro.c"],
    deps = [":%b_bolero_cdc_dlkm",
            ":%b_swr_ctrl_dlkm",
            ":%b_wcd_core_dlkm",
            ":%b_spf_core_dlkm",
           ],
)
audio_modules.register(
    name = "rx_macro_dlkm",
    path = ASOC_CODECS_BOLERO_PATH,
    config_option = "CONFIG_RX_MACRO",
    srcs = ["rx-macro.c"],
    deps = [":%b_bolero_cdc_dlkm",
            ":%b_swr_ctrl_dlkm",
            ":%b_wcd_core_dlkm",
           ],

)
audio_modules.register(
    name = "tx_macro_dlkm",
    path = ASOC_CODECS_BOLERO_PATH,
    config_option = "CONFIG_TX_MACRO",
    srcs = ["tx-macro.c"],
    deps = [":%b_bolero_cdc_dlkm",
            ":%b_swr_ctrl_dlkm",
            ":%b_wcd_core_dlkm",
           ],
)

# >>>> WSA881X-ANALOG MODULE <<<<
audio_modules.register(
    name = "wsa881x_analog_dlkm",
    path = ASOC_CODECS_PATH,
    config_option = "CONFIG_SND_SOC_WSA881X_ANALOG",
    srcs = [
        "wsa881x-analog.c",
        "wsa881x-tables-analog.c",
        "wsa881x-regmap-analog.c",
	],
    conditional_srcs = {
        "CONFIG_WSA881X_TEMP_SENSOR_DISABLE": {
            False: [
                "wsa881x-temp-sensor.c"
            ]
        }
    },
    deps = [":%b_wcd_core_dlkm",
            ":%b_swr_dlkm",
        ],
)

# >>>> WCD937X MODULES <<<<
audio_modules.register(
    name = "wcd937x_dlkm",
    path = ASOC_CODECS_PATH + "/wcd937x",
    config_option = "CONFIG_SND_SOC_WCD937X",
    srcs = [
        "wcd937x.c",
        "wcd937x-regmap.c",
        "wcd937x-tables.c",
        "wcd937x-mbhc.c",
    ],
    deps = [":%b_wcd9xxx_dlkm",
            ":%b_mbhc_dlkm",
            ":%b_wcd_core_dlkm",
            ":%b_swr_dlkm",
            ":%b_wcd937x_slave_dlkm",
	   ],
)
audio_modules.register(
    name = "wcd937x_slave_dlkm",
    path = ASOC_CODECS_PATH + "/wcd937x",
    config_option = "CONFIG_SND_SOC_WCD937X_SLAVE",
    srcs = ["wcd937x_slave.c"],
    deps = [":%b_swr_dlkm"],
)

# >>>> ROULEUR MODULE <<<<
audio_modules.register(
    name = "rouleur_dlkm",
    path = ASOC_CODECS_PATH + "/rouleur",
    config_option = "CONFIG_SND_SOC_ROULEUR",
    srcs = [
        "rouleur.c",
        "rouleur-regmap.c",
        "rouleur-tables.c",
        "rouleur-mbhc.c",
    ],
    deps = [":%b_rouleur_slave_dlkm",
            ":%b_swr_dlkm",
            ":%b_bolero_cdc_dlkm",
            ":%b_pm2250_spmi_dlkm",
            ":%b_wcd9xxx_dlkm",
            ":%b_wcd_core_dlkm",
            ":%b_mbhc_dlkm",
           ],
)
audio_modules.register(
    name = "rouleur_slave_dlkm",
    path = ASOC_CODECS_PATH + "/rouleur",
    config_option = "CONFIG_SND_SOC_ROULEUR_SLAVE",
    srcs = ["rouleur_slave.c"],
    deps = [":%b_swr_dlkm"],
)
audio_modules.register(
    name = "pm2250_spmi_dlkm",
    path = ASOC_CODECS_PATH + "/rouleur",
    config_option = "CONFIG_PM2250_SPMI",
    srcs = ["pm2250_spmi.c"],
)
