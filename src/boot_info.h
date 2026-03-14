#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "types.h"

#define BOOT_INFO_MAGIC 0xB007B007u
#define BOOT_INFO_VERSION 1u
#define MAX_MEMORY_MAP_ENTRIES 64

typedef struct {
  u64 base;
  u64 length;
  u32 type;
  u32 acpi;
} memory_map_entry_t;

typedef struct {
  u32 magic;
  u32 version;
  u32 memory_map_entries;
  u32 reserved0;
  u64 memory_map_addr;
  u64 kernel_phys_start;
  u64 kernel_phys_end;
  u64 kernel_virt_start;
  u64 pml4_addr;
} boot_info_t;

#endif
