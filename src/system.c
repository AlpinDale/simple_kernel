#include "system.h"
#include "cpu.h"
#include "io.h"

static volatile u64 timer_ticks = 0;

void system_reboot(void) {
  u8 temp;

  cpu_cli();

  do {
    temp = inb(0x64);
    if (temp & 0x01) {
      inb(0x60);
    }
  } while (temp & 0x02);

  outb(0x64, 0xFE);

  for (;;) {
    cpu_hlt();
  }
}

void system_halt(void) {
  cpu_cli();
  for (;;) {
    cpu_hlt();
  }
}

void system_timer_tick(void) { timer_ticks++; }

void system_init_timer(void) {
  u16 divisor = 1193;

  outb(0x43, 0x36);
  outb(0x40, divisor & 0xFF);
  outb(0x40, (divisor >> 8) & 0xFF);

  timer_ticks = 0;
}

u64 system_get_uptime_ms(void) { return timer_ticks; }

void system_sleep_ms(u64 delay_ms) {
  u64 deadline = system_get_uptime_ms() + delay_ms;

  while (system_get_uptime_ms() < deadline) {
    cpu_hlt();
  }
}

void system_qemu_exit(u16 code) { outb(0xF4, (u8)code); }
