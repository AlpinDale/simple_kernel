#include "cpu.h"
#include "cpuinfo.h"
#include "kstring.h"

void cpu_info_read(cpu_info_t *info) {
  if (info == 0) {
    return;
  }

  kmemset(info, 0, sizeof(*info));

  u32 eax = 0;
  u32 ebx = 0;
  u32 ecx = 0;
  u32 edx = 0;

  cpu_cpuid(0, 0, &eax, &ebx, &ecx, &edx);
  ((u32 *)info->vendor)[0] = ebx;
  ((u32 *)info->vendor)[1] = edx;
  ((u32 *)info->vendor)[2] = ecx;
  info->vendor[12] = '\0';

  cpu_cpuid(1, 0, &eax, &ebx, &ecx, &edx);

  u32 base_family = (eax >> 8) & 0xF;
  u32 ext_family = (eax >> 20) & 0xFF;
  u32 base_model = (eax >> 4) & 0xF;
  u32 ext_model = (eax >> 16) & 0xF;

  info->family = base_family;
  if (base_family == 0xF) {
    info->family += ext_family;
  }

  info->model = base_model;
  if (base_family == 0x6 || base_family == 0xF) {
    info->model |= ext_model << 4;
  }

  info->stepping = eax & 0xF;
  info->apic_present = ((edx >> 9) & 1u) != 0;
  info->msr_supported = ((edx >> 5) & 1u) != 0;
}
