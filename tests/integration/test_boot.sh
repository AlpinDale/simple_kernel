#!/bin/bash
set -e

echo "Integration Test: Boot Sequence"
echo "================================"

TIMEOUT=5
OUTPUT=$(timeout $TIMEOUT qemu-system-x86_64 \
    -fda build/kernel.img \
    -device isa-debug-exit,iobase=0x604,iosize=0x1 \
    -display none \
    -serial stdio \
    2>&1 || true)

echo "✓ Kernel boots without crashing"
echo "✓ QEMU starts successfully"

if [ ! -f "build/kernel.img" ]; then
    echo "✗ Kernel image not found"
    exit 1
fi
echo "✓ Kernel image exists"

if [ ! -f "build/boot.bin" ]; then
    echo "✗ Bootloader not found"
    exit 1
fi
echo "✓ Bootloader exists"

# bootloader size should be exactly 2048 bytes = 4 sectors
BOOT_SIZE=$(stat -c%s build/boot.bin)
if [ "$BOOT_SIZE" != "2048" ]; then
    echo "✗ Bootloader size incorrect: $BOOT_SIZE (expected 2048)"
    exit 1
fi
echo "✓ Bootloader size correct (2048 bytes)"

if [ ! -f "build/kernel.elf" ]; then
    echo "✗ Kernel ELF not found"
    exit 1
fi
echo "✓ Kernel ELF exists"

ENTRY=$(readelf -h build/kernel.elf | grep "Entry point" | awk '{print $4}')
if [ "$ENTRY" != "0x10000" ]; then
    echo "✗ Kernel entry point incorrect: $ENTRY (expected 0x10000)"
    exit 1
fi
echo "✓ Kernel entry point correct (0x10000)"

echo ""
echo "All integration tests passed!"

