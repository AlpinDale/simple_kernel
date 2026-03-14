#include "../test.h"

#include "../../src/boot_info.h"
#include "../../src/pmm.h"

static memory_map_entry_t entries[] = {
    {0x00000000, 0x0009F000, 1, 0},
    {0x00100000, 0x03F00000, 1, 0},
};

TEST(pmm_reserves_kernel_range) {
  boot_info_t boot_info = {0};
  boot_info.magic = BOOT_INFO_MAGIC;
  boot_info.memory_map_entries = 2;
  boot_info.memory_map_addr = (unsigned long long)(uintptr_t)entries;
  boot_info.kernel_phys_start = 0x00100000;
  boot_info.kernel_phys_end = 0x00120000;

  pmm_init(&boot_info);
  ASSERT_TRUE(pmm_free_bytes() < pmm_total_bytes());
  TEST_PASS_MSG();
}

TEST(pmm_allocates_frame) {
  boot_info_t boot_info = {0};
  boot_info.magic = BOOT_INFO_MAGIC;
  boot_info.memory_map_entries = 2;
  boot_info.memory_map_addr = (unsigned long long)(uintptr_t)entries;
  boot_info.kernel_phys_start = 0x00100000;
  boot_info.kernel_phys_end = 0x00120000;

  pmm_init(&boot_info);
  void *frame = pmm_alloc_frame();
  ASSERT_NOT_NULL(frame);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nPMM Tests:\n");
  RUN_TEST(pmm_reserves_kernel_range);
  RUN_TEST(pmm_allocates_frame);
  TEST_SUMMARY();
}
