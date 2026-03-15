#include "boot.h"

static const boot_info_t *current_boot_info;

void boot_init(const boot_info_t *boot_info) { current_boot_info = boot_info; }

const boot_info_t *boot_get_info(void) { return current_boot_info; }

const memory_map_entry_t *boot_get_memory_map(u32 *count) {
  if (current_boot_info == 0) {
    if (count != 0) {
      *count = 0;
    }
    return 0;
  }

  if (count != 0) {
    *count = current_boot_info->memory_map_entries;
  }

  return (const memory_map_entry_t *)(uintptr_t)current_boot_info->memory_map_addr;
}

const char *boot_memory_type_name(u32 type) {
  switch (type) {
  case 1:
    return "usable";
  case 2:
    return "reserved";
  case 3:
    return "acpi_reclaim";
  case 4:
    return "acpi_nvs";
  case 5:
    return "bad_memory";
  default:
    return "unknown";
  }
}
