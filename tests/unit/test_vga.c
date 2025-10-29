#include "../test.h"
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;

static inline u8 vga_entry_color(u8 fg, u8 bg) { return fg | bg << 4; }

static inline u16 vga_entry(unsigned char uc, u8 color) {
  return (u16)uc | (u16)color << 8;
}

TEST(vga_entry_color_basic) {
  u8 color = vga_entry_color(15, 0);
  ASSERT_EQ(color, 15);
  TEST_PASS_MSG();
}

TEST(vga_entry_color_with_background) {
  u8 color = vga_entry_color(15, 1);
  ASSERT_EQ(color, 0x1F);
  TEST_PASS_MSG();
}

TEST(vga_entry_creates_correct_value) {
  u16 entry = vga_entry('A', 0x0F);
  ASSERT_EQ(entry, 0x0F41);
  TEST_PASS_MSG();
}

TEST(vga_entry_color_components) {
  u8 color = vga_entry_color(7, 4);
  u8 fg = color & 0x0F;
  u8 bg = (color >> 4) & 0x0F;
  ASSERT_EQ(fg, 7);
  ASSERT_EQ(bg, 4);
  TEST_PASS_MSG();
}

TEST(vga_entry_extracts_char) {
  u16 entry = vga_entry('Z', 0x0F);
  u8 chr = entry & 0xFF;
  ASSERT_EQ(chr, 'Z');
  TEST_PASS_MSG();
}

TEST(vga_entry_extracts_color) {
  u16 entry = vga_entry('X', 0x2A);
  u8 color = (entry >> 8) & 0xFF;
  ASSERT_EQ(color, 0x2A);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nVGA Tests:\n");

  RUN_TEST(vga_entry_color_basic);
  RUN_TEST(vga_entry_color_with_background);
  RUN_TEST(vga_entry_creates_correct_value);
  RUN_TEST(vga_entry_color_components);
  RUN_TEST(vga_entry_extracts_char);
  RUN_TEST(vga_entry_extracts_color);

  TEST_SUMMARY();
}
