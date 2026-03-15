#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"

#define KERNEL_VERSION "0.2.0"

void system_reboot(void);
void system_halt(void);
void system_init_timer(void);
void system_timer_tick(void);
u64 system_get_uptime_ms(void);
void system_sleep_ms(u64 delay_ms);
void system_qemu_exit(u16 code);

#endif
