#ifndef PMM_H
#define PMM_H

#include "boot_info.h"
#include "types.h"

void pmm_init(const boot_info_t *boot_info);
void *pmm_alloc_frame(void);
void *pmm_alloc_frames(size_t count);
void pmm_free_frame(void *frame);
void pmm_free_frames(void *frame, size_t count);
u64 pmm_total_bytes(void);
u64 pmm_free_bytes(void);
u64 pmm_reserved_bytes(void);

#endif
