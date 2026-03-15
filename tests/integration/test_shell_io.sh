#!/usr/bin/env bash
set -euo pipefail

tests/integration/qemu_runner.sh \
  --image "build/kernel.img" \
  --script $'help\necho first\necho second\n<UP>\n\n<UP>\n<UP>\n<DOWN>\n\ncpuinfo\ntime\nsleep 5\npci\nmeminfo\nheapinfo\nallocdemo 128\nmmap\necho hello world\nexit\n' \
  --expect "Available commands:" \
  --expect "first" \
  --expect "second" \
  --expect "CPU vendor:" \
  --expect "RTC time:" \
  --expect "Sleeping for 5 ms..." \
  --expect "Awake" \
  --expect "PCI devices:" \
  --expect "PMM total bytes:" \
  --expect "Heap total bytes:" \
  --expect "Allocated 128 bytes at" \
  --expect "[0] base=" \
  --expect "hello world" \
  --expect-exit 33
