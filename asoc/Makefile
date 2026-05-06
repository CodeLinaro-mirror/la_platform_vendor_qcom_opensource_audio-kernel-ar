obj-$(CONFIG_SND_SOC_BENGAL) += bengal.o
ccflags-y += $(if $(IS_AGATTI_DIVAR),-DIS_AGATTI_DIVAR=$(IS_AGATTI_DIVAR))

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) modules $(KBUILD_OPTIONS) VERBOSE=1
modules_install:
	$(MAKE) M=$(M) -C $(KERNEL_SRC) modules_install
clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(M) clean
