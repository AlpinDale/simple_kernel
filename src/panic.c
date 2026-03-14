#include "cpu.h"
#include "kprint.h"
#include "panic.h"
#include "system.h"

void panic(const char *message) {
  kprint_colored("\nPANIC: ", VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
  kprint(message);
  kprint("\n");
  system_qemu_exit(0x21);
  cpu_cli();
  for (;;) {
    cpu_hlt();
  }
}

void panicf(const char *prefix, u64 value) {
  kprint_colored("\nPANIC: ", VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
  kprint(prefix);
  kprint_hex(value);
  kprint("\n");
  system_qemu_exit(0x21);
  cpu_cli();
  for (;;) {
    cpu_hlt();
  }
}
