#ifndef CPUINFO_H
#define CPUINFO_H

#include "types.h"

typedef struct {
  char vendor[13];
  u32 family;
  u32 model;
  u32 stepping;
  u8 apic_present;
  u8 msr_supported;
} cpu_info_t;

void cpu_info_read(cpu_info_t *info);

#endif
