BIN_DIR := bin
SRC_DIR := src

BCC ?= clang

ARCH_SOURCES := $(shell find src/arch/$(TARGET) -name '*.c')
ARCH_HEADERS := $(shell find src/arch/$(TARGET) -name '*.h')
CORE_SOURCES := $(shell find src/core/          -name '*.c')
CORE_HEADERS := $(shell find src/core/          -name '*.h')
OBJECTS := $(patsubst src/%,bin/%, $(patsubst %.c,%.c.o,$(ARCH_SOURCES) $(CORE_SOURCES)))

CFLAGS := -ffreestanding \
 	      -fshort-wchar \
 	      -Wno-unused-command-line- \
 	      -Wno-void-pointer-to-int-cast \
 	      -Wno-int-to-void-pointer-cast \
 	      -Wno-int-to-pointer-cast \
		  -Isrc \
		  -Ilib \
		  -DTARGET=$(TARGET)
LDFLAGS := -nostdlib -Wl,-entry:boot_entry -Wl,-subsystem:efi_application -fuse-ld=lld-link

CLANG_TARGET = x86_64-unknown-windows

OUT := $(BIN_DIR)/hboot.efi

all: $(OUT)

setup:
	@if [ ! -d "lib/efi" ]; then \
		mkdir -p lib; \
		cd lib && git clone https://github.com/aurixos/efi && cd ..; \
	fi
	@mkdir -p $(BIN_DIR)

$(OUT): $(OBJECTS)
	@printf "\tOUT\t\t$@\n"
	@$(BCC) $(OBJECTS) -o $@ -target $(CLANG_TARGET) $(LDFLAGS)

bin/%.c.o: src/%.c $(HEADERS)
	@printf "\t$(BCC)\t\t$@\n"
	@mkdir -p $(@D)
	@$(BCC) $(CFLAGS) -c $< -o $@ -target $(CLANG_TARGET) $(BCFLAGS) -I$(BOOT_DIR)/$(BEFILIBPATH)

clean:
	@rm -rf $(BIN_DIR)