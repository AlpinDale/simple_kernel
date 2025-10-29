CC = gcc
ASM = nasm
LD = ld
DD = dd

CFLAGS = -m64 -march=x86-64 -ffreestanding -nostdlib -Wall -Wextra -O2
ASMFLAGS = -f elf64
BOOT_ASMFLAGS = -f bin
LDFLAGS = -m elf_x86_64 -T linker.ld -nostdlib
BUILD_DIR = build

BOOTLOADER_SOURCE = boot.asm
KERNEL_ENTRY_SOURCE = kernel_entry.asm
KERNEL_ASM_SOURCES = idt_asm.asm
C_SOURCES = kernel.c vga.c idt.c keyboard.c

BOOTLOADER = $(BUILD_DIR)/boot.bin
KERNEL_ENTRY_OBJECT = $(BUILD_DIR)/kernel_entry.o
KERNEL_ASM_OBJECTS = $(KERNEL_ASM_SOURCES:%.asm=$(BUILD_DIR)/%.o)
C_OBJECTS = $(C_SOURCES:%.c=$(BUILD_DIR)/%.o)
KERNEL_OBJECTS = $(KERNEL_ENTRY_OBJECT) $(KERNEL_ASM_OBJECTS) $(C_OBJECTS)
KERNEL = $(BUILD_DIR)/kernel.bin
DISK_IMAGE = $(BUILD_DIR)/kernel.img

.PHONY: all clean run format

all: $(DISK_IMAGE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BOOTLOADER): $(BOOTLOADER_SOURCE) | $(BUILD_DIR)
	$(ASM) $(BOOT_ASMFLAGS) $< -o $@

$(KERNEL_ENTRY_OBJECT): $(KERNEL_ENTRY_SOURCE) | $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.asm | $(BUILD_DIR)
	$(ASM) $(ASMFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
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
	clang-format -i *.c *.h
	$(HOME)/go/bin/asmfmt -w *.asm 2>/dev/null || true

clean:
	rm -rf $(BUILD_DIR)
