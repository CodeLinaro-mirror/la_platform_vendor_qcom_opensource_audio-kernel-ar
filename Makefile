ifeq ($(TARGET_SUPPORT), monaco)
KBUILD_OPTIONS := CONFIG_SND_SOC_AUTO=y
KBUILD_OPTIONS += CONFIG_SND_SOC_SA7255=m
KBUILD_OPTIONS += MODNAME=audio_dlkm
endif

ifeq ($(AUTO_GVM), yes)
KBUILD_OPTIONS += CONFIG_SND_SOC_AUTO=y
KBUILD_OPTIONS += CONFIG_SND_SOC_GVM=y
KBUILD_OPTIONS += MODNAME=audio_dlkm
endif

ifeq ($(TARGET_SUPPORT), $(filter $(TARGET_SUPPORT), sdxpinn sdxpinn-cpe-wkk sdxbaagha))

ifeq ($(TARGET_SUPPORT), sdxpinn)
KBUILD_OPTIONS += CONFIG_ARCH_SDXPINN=y
endif

ifeq ($(TARGET_SUPPORT), sdxpinn-cpe-wkk)
KBUILD_OPTIONS += CONFIG_ARCH_SDXPINN=y
endif

endif

M=$(PWD)
AUDIO_ROOT=$(KERNEL_SRC)/$(M)

KBUILD_OPTIONS+=  AUDIO_ROOT=$(AUDIO_ROOT)

all: modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean

%:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) $@ $(KBUILD_OPTIONS)

