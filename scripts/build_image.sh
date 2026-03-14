#!/usr/bin/env bash
set -euo pipefail

SECTOR_SIZE=512
FLOPPY_SECTORS=2880

asm=""
stage1=""
stage2=""
kernel=""
boot1_out=""
boot2_out=""
image=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --asm)
      asm="$2"
      shift 2
      ;;
    --stage1)
      stage1="$2"
      shift 2
      ;;
    --stage2)
      stage2="$2"
      shift 2
      ;;
    --kernel)
      kernel="$2"
      shift 2
      ;;
    --boot1-out)
      boot1_out="$2"
      shift 2
      ;;
    --boot2-out)
      boot2_out="$2"
      shift 2
      ;;
    --image)
      image="$2"
      shift 2
      ;;
    *)
      echo "unknown argument: $1" >&2
      exit 1
      ;;
  esac
done

for required in "$asm" "$stage1" "$stage2" "$kernel" "$boot1_out" "$boot2_out" "$image"; do
  if [[ -z "$required" ]]; then
    echo "missing required argument" >&2
    exit 1
  fi
done

kernel_size=$(wc -c < "$kernel" | tr -d ' ')
kernel_sectors=$(((kernel_size + SECTOR_SIZE - 1) / SECTOR_SIZE))
stage2_tmp="${boot2_out%.bin}.tmp.bin"

"$asm" -f bin -DKERNEL_START_LBA=0 -DKERNEL_SECTORS="$kernel_sectors" -DKERNEL_SIZE_BYTES="$kernel_size" "$stage2" -o "$stage2_tmp"
stage2_tmp_size=$(wc -c < "$stage2_tmp" | tr -d ' ')
stage2_sectors=$(((stage2_tmp_size + SECTOR_SIZE - 1) / SECTOR_SIZE))

"$asm" -f bin -DKERNEL_START_LBA="$((1 + stage2_sectors))" -DKERNEL_SECTORS="$kernel_sectors" -DKERNEL_SIZE_BYTES="$kernel_size" "$stage2" -o "$boot2_out"
rm -f "$stage2_tmp"

stage2_size=$(wc -c < "$boot2_out" | tr -d ' ')
stage2_sectors=$(((stage2_size + SECTOR_SIZE - 1) / SECTOR_SIZE))

"$asm" -f bin -DSTAGE2_SECTORS="$stage2_sectors" "$stage1" -o "$boot1_out"

dd if=/dev/zero of="$image" bs=$SECTOR_SIZE count=$FLOPPY_SECTORS status=none
dd if="$boot1_out" of="$image" bs=$SECTOR_SIZE count=1 conv=notrunc status=none
dd if="$boot2_out" of="$image" bs=$SECTOR_SIZE seek=1 conv=notrunc status=none
dd if="$kernel" of="$image" bs=$SECTOR_SIZE seek=$((1 + stage2_sectors)) conv=notrunc status=none
