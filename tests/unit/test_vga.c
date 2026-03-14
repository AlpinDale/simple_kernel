#include "../test.h"

#include "../../src/vga.h"

static u16 mock_buffer[VGA_WIDTH * VGA_HEIGHT];

u8 test_inb(u16 port) {
  (void)port;
  return 0;
}

void test_outb(u16 port, u8 value) {
  (void)port;
  (void)value;
}

void test_outw(u16 port, u16 value) {
  (void)port;
  (void)value;
}

static void reset_vga(void) {
  vga_bind_buffer(mock_buffer);
  vga_initialize();
}

TEST(vga_entry_creates_correct_value) {
  ASSERT_EQ(vga_entry('A', 0x0F), 0x0F41);
  TEST_PASS_MSG();
}

TEST(vga_putchar_advances_cursor) {
  reset_vga();
  vga_putchar('A');
  ASSERT_EQ(mock_buffer[0] & 0xFF, 'A');
  ASSERT_EQ(vga_get_column(), 1);
  ASSERT_EQ(vga_get_row(), 0);
  TEST_PASS_MSG();
}

TEST(vga_scrolls_instead_of_wrapping) {
  reset_vga();

  for (int row = 0; row < VGA_HEIGHT; row++) {
    vga_putchar('0' + (row % 10));
    vga_putchar('\n');
  }

  vga_putchar('X');
  ASSERT_EQ(vga_get_row(), VGA_HEIGHT - 1);
  ASSERT_EQ(mock_buffer[0] & 0xFF, '1');
  ASSERT_EQ(mock_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH] & 0xFF, 'X');
  TEST_PASS_MSG();
}

TEST(vga_backspace_moves_to_previous_line) {
  reset_vga();
  vga_putchar('A');
  vga_putchar('\n');
  vga_putchar('B');
  vga_putchar('\b');
  ASSERT_EQ(vga_get_row(), 1);
  ASSERT_EQ(vga_get_column(), 0);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nVGA Tests:\n");
  RUN_TEST(vga_entry_creates_correct_value);
  RUN_TEST(vga_putchar_advances_cursor);
  RUN_TEST(vga_scrolls_instead_of_wrapping);
  RUN_TEST(vga_backspace_moves_to_previous_line);
  TEST_SUMMARY();
}
