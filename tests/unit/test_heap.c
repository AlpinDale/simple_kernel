#include "../test.h"

#include "../../src/heap.h"

static unsigned char backing_store[4096 * 8];
static size_t next_page;

void *pmm_alloc_frames(size_t count) {
  if (count == 0 || next_page + count > 8) {
    return 0;
  }

  void *ptr = &backing_store[next_page * 4096];
  next_page += count;
  return ptr;
}

void *pmm_alloc_frame(void) { return pmm_alloc_frames(1); }

void pmm_free_frame(void *frame) { (void)frame; }

void pmm_free_frames(void *frame, size_t count) {
  (void)frame;
  (void)count;
}

static void reset_heap(void) {
  next_page = 0;
  heap_init();
}

TEST(heap_allocates_and_frees_memory) {
  reset_heap();

  void *ptr = kmalloc(64);
  ASSERT_NOT_NULL(ptr);
  ASSERT_TRUE(heap_used_bytes() >= 64);

  kfree(ptr);
  ASSERT_EQ(heap_used_bytes(), 0);
  TEST_PASS_MSG();
}

TEST(heap_reuses_freed_blocks) {
  reset_heap();

  void *first = kmalloc(128);
  kfree(first);
  void *second = kmalloc(64);

  ASSERT_EQ(first, second);
  TEST_PASS_MSG();
}

TEST(kcalloc_returns_zeroed_memory) {
  reset_heap();

  unsigned char *ptr = kcalloc(16, sizeof(unsigned char));
  ASSERT_NOT_NULL(ptr);
  for (int i = 0; i < 16; i++) {
    ASSERT_EQ(ptr[i], 0);
  }
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nHeap Tests:\n");
  RUN_TEST(heap_allocates_and_frees_memory);
  RUN_TEST(heap_reuses_freed_blocks);
  RUN_TEST(kcalloc_returns_zeroed_memory);
  TEST_SUMMARY();
}
