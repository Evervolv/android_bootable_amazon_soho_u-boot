LOCAL_PATH := $(call my-dir)

ifeq ($(strip $(LOCAL_PATH)),$(TARGET_UBOOT_DIR))

INSTALLED_UBOOT_TARGET := $(PRODUCT_OUT)/u-boot.bin
UBOOT_OUT := $(abspath $(TARGET_OUT_INTERMEDIATES)/u-boot)
UBOOT_MAKE := $(MAKE) -C $(UBOOT_OUT) ARCH=arm CROSS_COMPILE=arm-eabi-

.PHONY: u-boot
u-boot: $(INSTALLED_UBOOT_TARGET)
all_modules: u-boot

$(INSTALLED_UBOOT_TARGET): $(UBOOT_OUT)/u-boot.bin
	$(hide) cp $< $@

$(UBOOT_OUT)/u-boot.bin: $(UBOOT_OUT)/include/config.h
	$(hide) $(UBOOT_MAKE)

$(UBOOT_OUT)/include/config.h: $(UBOOT_OUT)
	$(hide) $(UBOOT_MAKE) $(TARGET_UBOOT_CONFIG)

$(UBOOT_OUT): $(LOCAL_PATH)
	$(hide) cd $(TARGET_UBOOT_DIR); find . -type f \( -exec sh -c 'mkdir -p "$(UBOOT_OUT)/$$(dirname {})"; ln -sf "$(abspath $(TARGET_UBOOT_DIR))/{}" "$(UBOOT_OUT)/{}"' \; \)
	$(hide) rm $(UBOOT_OUT)/Android.mk

endif
