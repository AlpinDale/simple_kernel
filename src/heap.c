#include "heap.h"

#include "kstring.h"
#include "pmm.h"

#define HEAP_ALIGNMENT 16ull
#define HEAP_PAGE_SIZE 4096ull
#define HEAP_MIN_SPLIT 32ull

typedef struct heap_block {
  size_t size;
  struct heap_block *next;
  u8 free;
} heap_block_t;

static heap_block_t *heap_head;
static u64 total_bytes;
static u64 used_bytes;

static size_t align_up(size_t value) {
  return (value + HEAP_ALIGNMENT - 1) & ~(HEAP_ALIGNMENT - 1);
}

static void heap_append_block(heap_block_t *block) {
  if (heap_head == 0) {
    heap_head = block;
    return;
  }

  heap_block_t *current = heap_head;
  heap_block_t *prev = 0;
  uintptr_t target = (uintptr_t)block;

  while (current != 0 && (uintptr_t)current < target) {
    prev = current;
    current = current->next;
  }

  block->next = current;
  if (prev == 0) {
    heap_head = block;
  } else {
    prev->next = block;
  }
}

static void heap_coalesce(void) {
  heap_block_t *block = heap_head;

  while (block != 0 && block->next != 0) {
    uintptr_t block_end =
        (uintptr_t)(block + 1) + (uintptr_t)align_up(block->size);

    if (block->free && block->next->free &&
        block_end == (uintptr_t)block->next) {
      block->size += sizeof(heap_block_t) + block->next->size;
      block->next = block->next->next;
      continue;
    }

    block = block->next;
  }
}

static heap_block_t *heap_expand(size_t size) {
  size_t bytes = align_up(size + sizeof(heap_block_t));
  size_t pages = (bytes + HEAP_PAGE_SIZE - 1) / HEAP_PAGE_SIZE;
  void *region = pmm_alloc_frames(pages);

  if (region == 0) {
    return 0;
  }

  heap_block_t *block = (heap_block_t *)region;
  block->size = pages * HEAP_PAGE_SIZE - sizeof(heap_block_t);
  block->next = 0;
  block->free = 1;
  total_bytes += block->size;
  heap_append_block(block);
  heap_coalesce();
  return block;
}

static void heap_split_block(heap_block_t *block, size_t size) {
  size_t aligned_size = align_up(size);

  if (block->size < aligned_size + sizeof(heap_block_t) + HEAP_MIN_SPLIT) {
    return;
  }

  heap_block_t *next =
      (heap_block_t *)((u8 *)(block + 1) + aligned_size);
  next->size = block->size - aligned_size - sizeof(heap_block_t);
  next->next = block->next;
  next->free = 1;

  block->size = aligned_size;
  block->next = next;
}

void heap_init(void) {
  heap_head = 0;
  total_bytes = 0;
  used_bytes = 0;
}

void *kmalloc(size_t size) {
  if (size == 0) {
    return 0;
  }

  size = align_up(size);
  heap_block_t *block = heap_head;

  while (block != 0) {
    if (block->free && block->size >= size) {
      heap_split_block(block, size);
      block->free = 0;
      used_bytes += block->size;
      return (void *)(block + 1);
    }
    block = block->next;
  }

  block = heap_expand(size);
  if (block == 0) {
    return 0;
  }

  heap_split_block(block, size);
  block->free = 0;
  used_bytes += block->size;
  return (void *)(block + 1);
}

void *kcalloc(size_t count, size_t size) {
  if (count == 0 || size == 0) {
    return 0;
  }

  size_t total = count * size;
  void *ptr = kmalloc(total);
  if (ptr != 0) {
    kmemset(ptr, 0, total);
  }
  return ptr;
}

void kfree(void *ptr) {
  if (ptr == 0) {
    return;
  }

  heap_block_t *block = ((heap_block_t *)ptr) - 1;
  if (!block->free) {
    block->free = 1;
    used_bytes -= block->size;
    heap_coalesce();
  }
}

u64 heap_total_bytes(void) { return total_bytes; }

u64 heap_used_bytes(void) { return used_bytes; }

u64 heap_free_bytes(void) { return total_bytes - used_bytes; }
