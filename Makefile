ASM := nasm
HOST_CC ?= cc
BUILD_DIR := build
SRC_DIR := src
TESTS_DIR := tests
SCRIPTS_DIR := scripts
LLVM_PREFIX := $(shell brew --prefix llvm 2>/dev/null)

X86_GCC := $(shell command -v x86_64-elf-gcc 2>/dev/null)
X86_LD := $(shell command -v x86_64-elf-ld 2>/dev/null)
X86_OBJCOPY := $(shell command -v x86_64-elf-objcopy 2>/dev/null)
X86_READELF := $(shell command -v x86_64-elf-readelf 2>/dev/null)

ifeq ($(X86_GCC),)
CC := $(if $(wildcard $(LLVM_PREFIX)/bin/clang),$(LLVM_PREFIX)/bin/clang,clang)
LD := $(if $(wildcard $(LLVM_PREFIX)/bin/ld.lld),$(LLVM_PREFIX)/bin/ld.lld,$(if $(wildcard /opt/homebrew/opt/lld/bin/ld.lld),/opt/homebrew/opt/lld/bin/ld.lld,ld.lld))
OBJCOPY := $(if $(wildcard $(LLVM_PREFIX)/bin/llvm-objcopy),$(LLVM_PREFIX)/bin/llvm-objcopy,llvm-objcopy)
READELF := $(if $(wildcard $(LLVM_PREFIX)/bin/llvm-readelf),$(LLVM_PREFIX)/bin/llvm-readelf,llvm-readelf)
CFLAGS := --target=x86_64-unknown-none-elf -ffreestanding -fno-stack-protector -fno-pic -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -O2 -I$(SRC_DIR)
LDFLAGS := -m elf_x86_64 -T linker.ld -nostdlib
else
CC := $(X86_GCC)
LD := $(X86_LD)
OBJCOPY := $(X86_OBJCOPY)
READELF := $(X86_READELF)
CFLAGS := -ffreestanding -fno-stack-protector -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -O2 -I$(SRC_DIR)
LDFLAGS := -m elf_x86_64 -T linker.ld -nostdlib
endif

ASMFLAGS := -f elf64
BUILD_IMAGE := $(SCRIPTS_DIR)/build_image.sh

KERNEL_ENTRY_SOURCE := $(SRC_DIR)/kernel_entry.asm
KERNEL_ASM_SOURCES := $(SRC_DIR)/idt_asm.asm $(SRC_DIR)/gdt_asm.asm $(SRC_DIR)/trap_asm.asm
C_SOURCES := $(SRC_DIR)/kernel.c $(SRC_DIR)/console.c $(SRC_DIR)/gdt.c $(SRC_DIR)/idt.c \
	$(SRC_DIR)/input.c $(SRC_DIR)/keyboard.c $(SRC_DIR)/kprint.c $(SRC_DIR)/kstring.c \
	$(SRC_DIR)/panic.c $(SRC_DIR)/pmm.c $(SRC_DIR)/serial.c $(SRC_DIR)/shell.c \
	$(SRC_DIR)/system.c $(SRC_DIR)/trap.c $(SRC_DIR)/vga.c

BOOT_STAGE1_SOURCE := $(SRC_DIR)/boot_stage1.asm
BOOT_STAGE2_SOURCE := $(SRC_DIR)/boot_stage2.asm

KERNEL_ENTRY_OBJECT := $(BUILD_DIR)/kernel_entry.o
KERNEL_ASM_OBJECTS := $(KERNEL_ASM_SOURCES:$(SRC_DIR)/%.asm=$(BUILD_DIR)/%.o)
C_OBJECTS := $(C_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
KERNEL_OBJECTS := $(KERNEL_ENTRY_OBJECT) $(KERNEL_ASM_OBJECTS) $(C_OBJECTS)

KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BUILD_DIR)/kernel.bin
BOOT_STAGE1 := $(BUILD_DIR)/boot_stage1.bin
BOOT_STAGE2 := $(BUILD_DIR)/boot_stage2.bin
DISK_IMAGE := $(BUILD_DIR)/kernel.img

.PHONY: all clean doctor run test test-unit test-integration

all: doctor $(DISK_IMAGE)

doctor:
	@command -v $(ASM) >/dev/null || (echo "missing nasm" && exit 1)
	@command -v $(CC) >/dev/null || (echo "missing kernel compiler $(CC)" && exit 1)
	@command -v $(LD) >/dev/null || (echo "missing linker $(LD)" && exit 1)
	@command -v $(OBJCOPY) >/dev/null || (echo "missing objcopy $(OBJCOPY)" && exit 1)
	@command -v qemu-system-x86_64 >/dev/null || (echo "missing qemu-system-x86_64" && exit 1)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(KERNEL_ENTRY_OBJECT): $(KERNEL_ENTRY_SOURCE) | $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm | $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(KERNEL_OBJECTS) linker.ld | $(BUILD_DIR)
	$(LD) $(LDFLAGS) $(KERNEL_OBJECTS) -o $@

$(KERNEL_BIN): $(KERNEL_ELF) | $(BUILD_DIR)
	$(OBJCOPY) -O binary $< $@

$(DISK_IMAGE): $(BOOT_STAGE1_SOURCE) $(BOOT_STAGE2_SOURCE) $(KERNEL_BIN) $(SCRIPTS_DIR)/build_image.sh | $(BUILD_DIR)
	$(BUILD_IMAGE) --asm $(ASM) --stage1 $(BOOT_STAGE1_SOURCE) --stage2 $(BOOT_STAGE2_SOURCE) \
		--kernel $(KERNEL_BIN) --boot1-out $(BOOT_STAGE1) --boot2-out $(BOOT_STAGE2) --image $(DISK_IMAGE)

run: $(DISK_IMAGE)
	qemu-system-x86_64 -drive format=raw,file=$(DISK_IMAGE),if=ide,index=0 -device isa-debug-exit,iobase=0xF4,iosize=0x1

test: test-unit test-integration

test-unit: | $(BUILD_DIR)
	@echo "=== Running Unit Tests ==="
	@$(HOST_CC) -DKERNEL_TEST -I$(SRC_DIR) $(TESTS_DIR)/unit/test_vga.c $(SRC_DIR)/vga.c $(SRC_DIR)/kstring.c -o $(BUILD_DIR)/test_vga && ./$(BUILD_DIR)/test_vga
	@$(HOST_CC) -I$(SRC_DIR) $(TESTS_DIR)/unit/test_kstring.c $(SRC_DIR)/kstring.c -o $(BUILD_DIR)/test_kstring && ./$(BUILD_DIR)/test_kstring
	@$(HOST_CC) -DKERNEL_TEST -I$(SRC_DIR) $(TESTS_DIR)/unit/test_keyboard.c $(SRC_DIR)/keyboard.c -o $(BUILD_DIR)/test_keyboard && ./$(BUILD_DIR)/test_keyboard
	@$(HOST_CC) -I$(SRC_DIR) $(TESTS_DIR)/unit/test_keyboard_map.c -o $(BUILD_DIR)/test_keyboard_map && ./$(BUILD_DIR)/test_keyboard_map
	@$(HOST_CC) -I$(SRC_DIR) $(TESTS_DIR)/unit/test_idt.c $(SRC_DIR)/idt.c -o $(BUILD_DIR)/test_idt && ./$(BUILD_DIR)/test_idt
	@$(HOST_CC) -I$(SRC_DIR) $(TESTS_DIR)/unit/test_shell_parser.c $(SRC_DIR)/shell.c $(SRC_DIR)/kstring.c -o $(BUILD_DIR)/test_shell_parser && ./$(BUILD_DIR)/test_shell_parser
	@$(HOST_CC) -I$(SRC_DIR) $(TESTS_DIR)/unit/test_pmm.c $(SRC_DIR)/pmm.c $(SRC_DIR)/kstring.c -o $(BUILD_DIR)/test_pmm && ./$(BUILD_DIR)/test_pmm

test-integration: $(DISK_IMAGE)
	@echo "\n=== Running Integration Tests ==="
	@$(TESTS_DIR)/integration/test_boot.sh
	@$(TESTS_DIR)/integration/test_shell_io.sh
	@$(TESTS_DIR)/integration/test_panic.sh

clean:
	rm -rf $(BUILD_DIR)
