#!/usr/bin/env bash
set -euo pipefail

tests/integration/qemu_runner.sh \
  --image "build/kernel.img" \
  --script $'panic\n' \
  --expect "PANIC: panic command invoked" \
  --expect-exit 67
