#!/usr/bin/env bash
set -euo pipefail

tests/integration/qemu_runner.sh \
  --image "build/kernel.img" \
  --script $'help\nmeminfo\necho hello world\nexit\n' \
  --expect "Available commands:" \
  --expect "PMM total bytes:" \
  --expect "hello world" \
  --expect-exit 33
