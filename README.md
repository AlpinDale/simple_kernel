# simple x86_64 kernel

A small BIOS-booted x86_64 kernel with a two-stage
bootloader, boot-info handoff, IRQ and exception handling, serial/VGA console
output, a simple PMM, and a simple shell.

## Tooling

Supported hosts:

- Linux with LLVM + `lld` or a native `x86_64-elf-*` toolchain
- macOS with Homebrew `llvm`, `lld`, `nasm`, and `qemu`

macOS setup:

```bash
brew install llvm lld nasm qemu
```

Ubuntu setup:

```bash
sudo apt-get update
sudo apt-get install -y clang lld llvm nasm qemu-system-x86
```

## Build and test

```bash
make doctor
make
make test
```

## Run

```bash
make run
```
