CC = gcc
ASM = nasm
LD = ld
DD = dd

CFLAGS = -m64 -march=x86-64 -ffreestanding -nostdlib -Wall -Wextra -O2 -I$(SRC_DIR)
ASMFLAGS = -f elf64
BOOT_ASMFLAGS = -f bin
LDFLAGS = -m elf_x86_64 -T linker.ld -nostdlib
BUILD_DIR = build
SRC_DIR = src
TESTS_DIR = tests

BOOTLOADER_SOURCE = $(SRC_DIR)/boot.asm
KERNEL_ENTRY_SOURCE = $(SRC_DIR)/kernel_entry.asm
KERNEL_ASM_SOURCES = $(SRC_DIR)/idt_asm.asm
C_SOURCES = $(SRC_DIR)/kernel.c $(SRC_DIR)/vga.c $(SRC_DIR)/idt.c $(SRC_DIR)/keyboard.c

BOOTLOADER = $(BUILD_DIR)/boot.bin
KERNEL_ENTRY_OBJECT = $(BUILD_DIR)/kernel_entry.o
KERNEL_ASM_OBJECTS = $(KERNEL_ASM_SOURCES:$(SRC_DIR)/%.asm=$(BUILD_DIR)/%.o)
C_OBJECTS = $(C_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
KERNEL_OBJECTS = $(KERNEL_ENTRY_OBJECT) $(KERNEL_ASM_OBJECTS) $(C_OBJECTS)
KERNEL = $(BUILD_DIR)/kernel.bin
DISK_IMAGE = $(BUILD_DIR)/kernel.img

.PHONY: all clean run format test test-unit test-integration

all: $(DISK_IMAGE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BOOTLOADER): $(BOOTLOADER_SOURCE) | $(BUILD_DIR)
	$(ASM) $(BOOT_ASMFLAGS) $< -o $@

$(KERNEL_ENTRY_OBJECT): $(KERNEL_ENTRY_SOURCE) | $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm | $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

KERNEL_ELF = $(BUILD_DIR)/kernel.elf
$(KERNEL_ELF): $(KERNEL_OBJECTS) linker.ld
	$(LD) $(LDFLAGS) $(KERNEL_OBJECTS) -o $@

$(KERNEL): $(KERNEL_ELF)
	objcopy -O binary --set-section-flags .bss=alloc,load,contents $< $(BUILD_DIR)/kernel_raw.bin
	$(DD) if=/dev/zero of=$(BUILD_DIR)/kernel_pad.bin bs=1 count=65536 2>/dev/null
	$(DD) if=$(BUILD_DIR)/kernel_raw.bin of=$(BUILD_DIR)/kernel_pad.bin bs=1 seek=65536 conv=notrunc 2>/dev/null
	mv $(BUILD_DIR)/kernel_pad.bin $@

$(DISK_IMAGE): $(BOOTLOADER) $(KERNEL)
	$(DD) if=/dev/zero of=$(DISK_IMAGE) bs=512 count=2880 2>/dev/null
	$(DD) if=$(BOOTLOADER) of=$(DISK_IMAGE) bs=512 count=4 conv=notrunc 2>/dev/null
	$(DD) if=$(KERNEL) of=$(DISK_IMAGE) bs=512 skip=128 seek=4 conv=notrunc 2>/dev/null

run: $(DISK_IMAGE)
	qemu-system-x86_64 -fda $(DISK_IMAGE) -device isa-debug-exit,iobase=0x604,iosize=0x1

format:
	clang-format -i $(SRC_DIR)/*.c $(SRC_DIR)/*.h $(TESTS_DIR)/*.h $(TESTS_DIR)/unit/*.c
	$(HOME)/go/bin/asmfmt -w $(SRC_DIR)/*.asm 2>/dev/null || true

test: test-unit test-integration

test-unit: | $(BUILD_DIR)
	@echo "=== Running Unit Tests ==="
	@gcc $(TESTS_DIR)/unit/test_vga.c -o build/test_vga && ./build/test_vga
	@gcc $(TESTS_DIR)/unit/test_vga_driver.c -o build/test_vga_driver && ./build/test_vga_driver
	@gcc $(TESTS_DIR)/unit/test_idt.c -o build/test_idt && ./build/test_idt
	@gcc $(TESTS_DIR)/unit/test_keyboard.c -o build/test_keyboard && ./build/test_keyboard

test-integration: $(DISK_IMAGE)
	@echo "\n=== Running Integration Tests ==="
	@./$(TESTS_DIR)/integration/test_boot.sh
	@./$(TESTS_DIR)/integration/test_keyboard_input.sh

clean:
	rm -rf $(BUILD_DIR)
