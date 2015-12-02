UBOOT_DIR := $(call my-dir)
ROOTDIR := $(abspath $(TOP))

UBOOT_IMAGE_INTERMEDIATES := $(UBOOT_DIR)/u-boot.bin

UBOOT_IMAGE := $(PRODUCT_OUT)/u-boot.bin

UBOOT_CONFIG := $(UBOOT_DIR)/include/config.h

$(UBOOT_IMAGE): $(UBOOT_IMAGE_INTERMEDIATES) uboot-intermediates | $(ACP)
ifeq ($(PRODUCT_SIGN_BOOTLOADER),true)
	$(hide) $(PRODUCT_SIGN_TOOL) -k $(SIGN_KEY_TYPE) $(SIGN_CHIP) $(SIGN_CHIP_VERSION) $< -o $@
else
	$(copy-file-to-target)
endif
	$(hide) mkdir -p $(PRODUCT_OUT)/unsigned && cp -f $(UBOOT_IMAGE_INTERMEDIATES) $(PRODUCT_OUT)/unsigned/u-boot.unsigned.bin

$(UBOOT_IMAGE_INTERMEDIATES): uboot-intermediates | $(UBOOT_DIR)

uboot-config: $(UBOOT_CONFIG)

$(UBOOT_CONFIG): $(UBOOT_DEFCONFIG) | $(UBOOT_DIR)
	$(MAKE) -C $(UBOOT_DIR) distclean
	$(MAKE) -C $(UBOOT_DIR) ARCH=arm CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) $(UBOOT_DEFCONFIG) PLATFORM=$(UBOOT_PLATFORM)

uboot-intermediates: uboot-config | $(UBOOT_DIR)
ifeq ($(TARGET_BUILD_VARIANT), user)
	$(MAKE) -C $(UBOOT_DIR) ARCH=arm CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) PLATFORM=$(UBOOT_PLATFORM) -j1
else
	$(MAKE) -C $(UBOOT_DIR) ARCH=arm CROSS_COMPILE=$(UBOOT_CROSS_COMPILE) PLATFORM=$(UBOOT_PLATFORM) EXTRA_CFLAGS=-DUBOOT_ENG_BUILD -j1
endif

uboot: $(UBOOT_IMAGE)
