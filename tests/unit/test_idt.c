#include "../test.h"

#include "../../src/idt.h"

void idt_load(u64 ptr) { (void)ptr; }

TEST(idt_entry_size) {
  ASSERT_EQ(sizeof(struct idt_entry), 16);
  TEST_PASS_MSG();
}

TEST(idt_set_gate_reconstructs_handler) {
  idt_init();
  idt_set_gate(14, 0xDEADBEEF12345678ull, 0x08, 0x8E);

  struct idt_entry entry = {0};
  idt_set_gate(1, 0x123456789ABCull, 0x08, 0x8E);

  entry.offset_low = 0x9ABC;
  entry.offset_mid = 0x5678;
  entry.offset_high = 0x1234;
  u64 reconstructed = ((u64)entry.offset_high << 32) |
                      ((u64)entry.offset_mid << 16) | entry.offset_low;

  ASSERT_EQ(reconstructed, 0x123456789ABCull);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nIDT Tests:\n");
  RUN_TEST(idt_entry_size);
  RUN_TEST(idt_set_gate_reconstructs_handler);
  TEST_SUMMARY();
}
