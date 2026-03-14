#!/usr/bin/env bash
set -euo pipefail

image="build/kernel.img"
if [[ ! -f "$image" ]]; then
  echo "kernel image not found" >&2
  exit 1
fi

tests/integration/qemu_runner.sh \
  --image "$image" \
  --script $'exit\n' \
  --expect "simple_kernel 0.2.0" \
  --expect "boot_info memory entries:" \
  --expect "kernel> " \
  --expect-exit 33
