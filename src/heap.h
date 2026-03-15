#ifndef HEAP_H
#define HEAP_H

#include "types.h"

void heap_init(void);
void *kmalloc(size_t size);
void *kcalloc(size_t count, size_t size);
void kfree(void *ptr);
u64 heap_total_bytes(void);
u64 heap_used_bytes(void);
u64 heap_free_bytes(void);

#endif
