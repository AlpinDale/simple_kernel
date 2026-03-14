#ifndef PMM_H
#define PMM_H

#include "boot_info.h"
#include "types.h"

void pmm_init(const boot_info_t *boot_info);
void *pmm_alloc_frame(void);
void pmm_free_frame(void *frame);
u64 pmm_total_bytes(void);
u64 pmm_free_bytes(void);
u64 pmm_reserved_bytes(void);

#endif
