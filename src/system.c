#include "system.h"

static volatile u64 timer_ticks = 0;

static inline void outb(u16 port, u8 value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {
  u8 ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void system_reboot(void) {
  u8 temp;

  __asm__ volatile("cli");

  do {
    temp = inb(0x64);
    if (temp & 0x01) {
      inb(0x60);
    }
  } while (temp & 0x02);

  outb(0x64, 0xFE);

  for (;;) {
    __asm__ volatile("hlt");
  }
}

void system_halt(void) {
  __asm__ volatile("cli");
  for (;;) {
    __asm__ volatile("hlt");
  }
}

void timer_handler(void) {
  timer_ticks++;
  outb(0x20, 0x20); // EOI
}

void system_init_timer(void) {
  // Set PIT to ~1000 Hz (1ms intervals)
  u16 divisor = 1193; // 1193182 Hz / 1193 ≈ 1000 Hz

  outb(0x43, 0x36); // Command: channel 0, lo/hi byte, rate generator
  outb(0x40, divisor & 0xFF);
  outb(0x40, (divisor >> 8) & 0xFF);

  timer_ticks = 0;
}

u64 system_get_uptime_ms(void) { return timer_ticks; }
