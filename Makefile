include .config

KERNEL_CONFIG_PATH := src/config.h
KCONFIGLIB_URL := https://github.com/ulfalizer/Kconfiglib/archive/refs/heads/master.zip
KCONFIGLIB_ZIP := kconfiglib.zip
KCONFIGLIB_DIR := kconfiglib

# Target to download and extract Kconfiglib if needed
$(KCONFIGLIB_DIR):
	@if [ ! -d "$(KCONFIGLIB_DIR)" ]; then \
		echo "Kconfiglib not found. Downloading..."; \
		curl -L -o $(KCONFIGLIB_ZIP) $(KCONFIGLIB_URL); \
		unzip $(KCONFIGLIB_ZIP) -d $(KCONFIGLIB_DIR); \
		rm $(KCONFIGLIB_ZIP); \
	fi

all: setup TARGET_CHECK

.config: $(KCONFIGLIB_DIR)
	@$(KCONFIGLIB_DIR)/Kconfiglib-master/alldefconfig.py
	@$(MAKE) $(KERNEL_CONFIG_PATH)

$(KERNEL_CONFIG_PATH): Kconfig
	@$(KCONFIGLIB_DIR)/Kconfiglib-master/genconfig.py --header-path $@

.PHONY: menuconfig
menuconfig: $(KCONFIGLIB_DIR)
	@$(KCONFIGLIB_DIR)/Kconfiglib-master/menuconfig.py
	@$(MAKE) $(KERNEL_CONFIG_PATH)

.PHONY: guiconfig
guiconfig: $(KCONFIGLIB_DIR)
	@$(KCONFIGLIB_DIR)/guiconfig.py
	@$(MAKE) $(KERNEL_CONFIG_PATH)

TARGET_CHECK:
	@if [ ! -f src/arch/$(TARGET)/build.mk ]; then \
		echo "Error: TARGET file src/arch/$(TARGET)/build.mk does not exist."; \
		exit 1; \
	fi

include src/arch/$(TARGET)/build.mk

.PHONY: cleandist
cleandist:
	@rm -f $(KERNEL_CONFIG_PATH) .config