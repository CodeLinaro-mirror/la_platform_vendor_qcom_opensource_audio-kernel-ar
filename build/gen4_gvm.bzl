load(":audio_modules.bzl", "audio_modules")
load(":module_mgr.bzl", "define_target_modules")

def define_gen4_gvm():
    define_target_modules(
        target = "autogvm",
        variants = ["consolidate", "perf"],
        registry = audio_modules,
        modules = [
            "spf_machine_dlkm",
            "stub_dlkm",
        ],
        config_options = [
           "CONFIG_SND_SOC_GVM_AUTO_SPF",
        ],
    )