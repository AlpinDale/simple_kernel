#ifndef SYSTEM_H
#define SYSTEM_H

#include "types.h"

#define KERNEL_VERSION "0.1.0"

void system_reboot(void);
void system_halt(void);
void system_init_timer(void);
u64 system_get_uptime_ms(void);

#endif
