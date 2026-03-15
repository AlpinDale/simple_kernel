#include "pmm.h"
#include "kstring.h"

#define PMM_PAGE_SIZE 4096ull
#define PMM_MAX_MEMORY (1ull << 30)
#define PMM_MAX_FRAMES (PMM_MAX_MEMORY / PMM_PAGE_SIZE)

static u8 frame_bitmap[PMM_MAX_FRAMES / 8];
static u64 frame_limit = PMM_MAX_FRAMES;
static u64 total_frames;
static u64 free_frames;

static void bit_set(u64 frame) { frame_bitmap[frame / 8] |= (u8)(1u << (frame % 8)); }

static void bit_clear(u64 frame) {
  frame_bitmap[frame / 8] &= (u8)~(1u << (frame % 8));
}

static u8 bit_test(u64 frame) {
  return (frame_bitmap[frame / 8] & (u8)(1u << (frame % 8))) != 0;
}

static u64 page_align_up(u64 value) {
  return (value + PMM_PAGE_SIZE - 1) & ~(PMM_PAGE_SIZE - 1);
}

static u64 page_align_down(u64 value) { return value & ~(PMM_PAGE_SIZE - 1); }

static void reserve_range(u64 base, u64 length) {
  u64 start = page_align_down(base);
  u64 end = page_align_up(base + length);

  if (start >= PMM_MAX_MEMORY) {
    return;
  }
  if (end > PMM_MAX_MEMORY) {
    end = PMM_MAX_MEMORY;
  }

  for (u64 addr = start; addr < end; addr += PMM_PAGE_SIZE) {
    u64 frame = addr / PMM_PAGE_SIZE;
    if (!bit_test(frame)) {
      bit_set(frame);
      free_frames--;
    }
  }
}

static void free_range(u64 base, u64 length) {
  u64 start = page_align_up(base);
  u64 end = page_align_down(base + length);

  if (start >= end || start >= PMM_MAX_MEMORY) {
    return;
  }
  if (end > PMM_MAX_MEMORY) {
    end = PMM_MAX_MEMORY;
  }

  for (u64 addr = start; addr < end; addr += PMM_PAGE_SIZE) {
    u64 frame = addr / PMM_PAGE_SIZE;
    if (bit_test(frame)) {
      bit_clear(frame);
      free_frames++;
    }
  }
}

void pmm_init(const boot_info_t *boot_info) {
  kmemset(frame_bitmap, 0xFF, sizeof(frame_bitmap));
  total_frames = frame_limit;
  free_frames = 0;

  const memory_map_entry_t *entries =
      (const memory_map_entry_t *)(uintptr_t)boot_info->memory_map_addr;

  for (u32 i = 0; i < boot_info->memory_map_entries; i++) {
    if (entries[i].type == 1) {
      free_range(entries[i].base, entries[i].length);
    }
  }

  reserve_range(0, 0x100000);
  reserve_range(boot_info->kernel_phys_start,
                boot_info->kernel_phys_end - boot_info->kernel_phys_start);
  reserve_range((u64)(uintptr_t)boot_info, sizeof(*boot_info));
  reserve_range(boot_info->memory_map_addr,
                sizeof(memory_map_entry_t) * boot_info->memory_map_entries);
  reserve_range(0x1000, 0x4000);
}

void *pmm_alloc_frame(void) {
  return pmm_alloc_frames(1);
}

void *pmm_alloc_frames(size_t count) {
  if (count == 0 || count > frame_limit) {
    return 0;
  }

  u64 run_start = 0;
  u64 run_length = 0;

  for (u64 frame = 0; frame < frame_limit; frame++) {
    if (!bit_test(frame)) {
      if (run_length == 0) {
        run_start = frame;
      }
      run_length++;

      if (run_length == count) {
        for (u64 i = 0; i < count; i++) {
          bit_set(run_start + i);
        }
        free_frames -= count;
        return (void *)(uintptr_t)(run_start * PMM_PAGE_SIZE);
      }
    } else {
      run_length = 0;
    }
  }

  return 0;
}

void pmm_free_frame(void *frame_ptr) {
  pmm_free_frames(frame_ptr, 1);
}

void pmm_free_frames(void *frame_ptr, size_t count) {
  if (frame_ptr == 0 || count == 0) {
    return;
  }

  u64 frame = (u64)(uintptr_t)frame_ptr / PMM_PAGE_SIZE;

  if (frame >= frame_limit) {
    return;
  }

  for (size_t i = 0; i < count; i++) {
    u64 current = frame + i;
    if (current >= frame_limit || !bit_test(current)) {
      break;
    }
    bit_clear(current);
    free_frames++;
  }
}

u64 pmm_total_bytes(void) { return total_frames * PMM_PAGE_SIZE; }

u64 pmm_free_bytes(void) { return free_frames * PMM_PAGE_SIZE; }

u64 pmm_reserved_bytes(void) { return pmm_total_bytes() - pmm_free_bytes(); }
