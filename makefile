BIN_DIR := bin
COMMON_DIR := common

TARGET_COMMON := $(BIN_DIR)/BOOTX64.elf

.PHONY: all
all: setup $(TARGET_COMMON)

$(TARGET_COMMON): $(COMMON_DIR)/branch.efi | $(BIN_DIR)
	@cp $(COMMON_DIR)/branch.efi $(TARGET_COMMON)

$(COMMON_DIR)/branch.efi: common
	@$(MAKE) -C $(COMMON_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

.PHONY: setup
setup: $(BIN_DIR)

.PHONY: common
common:
	@$(MAKE) -C $(COMMON_DIR)

.PHONY: test
test: | $(TARGET_COMMON)
	@curl --output ovmf.fd https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd
	@if [ "$(shell uname -s)" = "Darwin" ]; then \
	    dd if=/dev/zero of=boot.img bs=1m count=64; \
	    mkfs.fat -F 32 -n EFI_SYSTEM boot.img; \
	    mmd -i boot.img ::/EFI ::/EFI/BOOT; \
	    mcopy -i boot.img $(TARGET_COMMON) ::/EFI/BOOT/BOOTX64.efi; \
		mcopy -i boot.cfg ::boot.cfg; \
	else \
	    dd if=/dev/zero of=boot.img bs=1M count=64; \
	    mkfs.fat -F 32 -n EFI_SYSTEM boot.img; \
	    mkdir -p mnt; \
	    sudo mount -o loop boot.img mnt; \
	    sudo mkdir -p mnt/EFI/BOOT; \
	    sudo cp $(TARGET_COMMON) mnt/EFI/BOOT/BOOTX64.efi; \
		sudo cp boot.cfg mnt/boot.cfg; \
	    sudo umount mnt; \
	    rm -rf mnt; \
	fi
	@qemu-system-x86_64 -m 2G -drive if=pflash,format=raw,readonly=on,file=ovmf.fd -drive if=ide,format=raw,file=boot.img -serial stdio

.PHONY: clean
clean:
	@$(MAKE) -C $(COMMON_DIR) clean
	@rm -rf $(BIN_DIR) mnt boot.img ovmf.fd