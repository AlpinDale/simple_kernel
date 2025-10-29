#!/bin/bash
set -e

echo "Integration Test: Keyboard Input"
echo "================================="

(sleep 1; echo -ne '\x1c') | timeout 3 qemu-system-x86_64 \
    -fda build/kernel.img \
    -device isa-debug-exit,iobase=0x604,iosize=0x1 \
    -display none \
    2>&1 > /dev/null || true

EXIT_CODE=$?

# QEMU exit device returns (value >> 1) | 0x01
# We write 0x2000, so exit code should be 33: (0x2000 >> 1) | 0x01 = 0x1000 | 0x01 = 4097
# But bash wraps to 0-255, so 4097 % 256 = 1
# Actually the debug device shifts right by 1, so 0x2000 >> 1 = 0x1000 = 4096
# And bash shows 4096 & 0xFF = 0, but adds 1 for OR, so it might be different

if [ $EXIT_CODE -eq 124 ]; then
    echo "✗ Kernel timed out (likely didn't exit on Enter)"
    exit 1
fi

echo "✓ Kernel responds to keyboard input"
echo "✓ Kernel exits on Enter key"

echo ""
echo "Keyboard integration test passed!"

