# simple x86_64 kernel


## Build & Run (arch linux)

```bash
sudo pacman -S base-devel nasm qemu
```

```bash
make run
```


## What does it do?

Boots with a two-stage bootloader, transitions to 64-bit long mode, and inits keyboard interrupts. Prints "Hello World", waits for Enter, then exits QEMU.
