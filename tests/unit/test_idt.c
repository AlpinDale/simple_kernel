#include "../test.h"
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct idt_entry {
  u16 offset_low;
  u16 selector;
  u8 ist;
  u8 type_attr;
  u16 offset_mid;
  u32 offset_high;
  u32 zero;
} __attribute__((packed));

static void idt_set_gate_impl(struct idt_entry *entry, u64 handler,
                              u16 selector, u8 flags) {
  entry->offset_low = handler & 0xFFFF;
  entry->selector = selector;
  entry->ist = 0;
  entry->type_attr = flags;
  entry->offset_mid = (handler >> 16) & 0xFFFF;
  entry->offset_high = (handler >> 32) & 0xFFFFFFFF;
  entry->zero = 0;
}

TEST(idt_entry_size) {
  ASSERT_EQ(sizeof(struct idt_entry), 16);
  TEST_PASS_MSG();
}

TEST(idt_set_gate_offset_low) {
  struct idt_entry entry = {0};
  idt_set_gate_impl(&entry, 0x123456789ABCULL, 0x08, 0x8E);
  ASSERT_EQ(entry.offset_low, 0x9ABC);
  TEST_PASS_MSG();
}

TEST(idt_set_gate_offset_mid) {
  struct idt_entry entry = {0};
  idt_set_gate_impl(&entry, 0x123456789ABCULL, 0x08, 0x8E);
  ASSERT_EQ(entry.offset_mid, 0x5678);
  TEST_PASS_MSG();
}

TEST(idt_set_gate_offset_high) {
  struct idt_entry entry = {0};
  idt_set_gate_impl(&entry, 0x123456789ABCULL, 0x08, 0x8E);
  ASSERT_EQ(entry.offset_high, 0x1234);
  TEST_PASS_MSG();
}

TEST(idt_set_gate_selector) {
  struct idt_entry entry = {0};
  idt_set_gate_impl(&entry, 0x1000, 0x08, 0x8E);
  ASSERT_EQ(entry.selector, 0x08);
  TEST_PASS_MSG();
}

TEST(idt_set_gate_type_attr) {
  struct idt_entry entry = {0};
  idt_set_gate_impl(&entry, 0x1000, 0x08, 0x8E);
  ASSERT_EQ(entry.type_attr, 0x8E);
  TEST_PASS_MSG();
}

TEST(idt_set_gate_ist_is_zero) {
  struct idt_entry entry = {0};
  idt_set_gate_impl(&entry, 0x1000, 0x08, 0x8E);
  ASSERT_EQ(entry.ist, 0);
  TEST_PASS_MSG();
}

TEST(idt_set_gate_zero_field) {
  struct idt_entry entry = {0};
  idt_set_gate_impl(&entry, 0x1000, 0x08, 0x8E);
  ASSERT_EQ(entry.zero, 0);
  TEST_PASS_MSG();
}

TEST(idt_reconstruct_handler_address) {
  struct idt_entry entry = {0};
  u64 original = 0xDEADBEEF12345678ULL;
  idt_set_gate_impl(&entry, original, 0x08, 0x8E);

  u64 reconstructed = ((u64)entry.offset_high << 32) |
                      ((u64)entry.offset_mid << 16) | entry.offset_low;

  ASSERT_EQ(reconstructed, original);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nIDT Tests:\n");

  RUN_TEST(idt_entry_size);
  RUN_TEST(idt_set_gate_offset_low);
  RUN_TEST(idt_set_gate_offset_mid);
  RUN_TEST(idt_set_gate_offset_high);
  RUN_TEST(idt_set_gate_selector);
  RUN_TEST(idt_set_gate_type_attr);
  RUN_TEST(idt_set_gate_ist_is_zero);
  RUN_TEST(idt_set_gate_zero_field);
  RUN_TEST(idt_reconstruct_handler_address);

  TEST_SUMMARY();
}
