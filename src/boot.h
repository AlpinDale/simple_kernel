#ifndef BOOT_H
#define BOOT_H

#include "boot_info.h"

void boot_init(const boot_info_t *boot_info);
const boot_info_t *boot_get_info(void);
const memory_map_entry_t *boot_get_memory_map(u32 *count);
const char *boot_memory_type_name(u32 type);

#endif
