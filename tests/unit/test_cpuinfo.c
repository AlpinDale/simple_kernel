#include "../test.h"

#include "../../src/cpuinfo.h"

void test_cpuid(unsigned int leaf, unsigned int subleaf, unsigned int *eax,
                unsigned int *ebx, unsigned int *ecx, unsigned int *edx) {
  (void)subleaf;

  if (leaf == 0) {
    *eax = 1;
    *ebx = 0x756E6547;
    *edx = 0x49656E69;
    *ecx = 0x6C65746E;
    return;
  }

  if (leaf == 1) {
    *eax = 0x000906E9;
    *ebx = 0;
    *ecx = 0;
    *edx = (1u << 9) | (1u << 5);
    return;
  }

  *eax = 0;
  *ebx = 0;
  *ecx = 0;
  *edx = 0;
}

TEST(cpu_info_reads_vendor_string) {
  cpu_info_t info;
  cpu_info_read(&info);
  ASSERT_EQ(strcmp(info.vendor, "GenuineIntel"), 0);
  TEST_PASS_MSG();
}

TEST(cpu_info_decodes_family_model_and_flags) {
  cpu_info_t info;
  cpu_info_read(&info);
  ASSERT_EQ(info.family, 6);
  ASSERT_EQ(info.model, 0x9E);
  ASSERT_EQ(info.stepping, 9);
  ASSERT_TRUE(info.apic_present);
  ASSERT_TRUE(info.msr_supported);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nCPU Info Tests:\n");
  RUN_TEST(cpu_info_reads_vendor_string);
  RUN_TEST(cpu_info_decodes_family_model_and_flags);
  TEST_SUMMARY();
}
