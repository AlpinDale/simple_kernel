#include "../test.h"
#include <stdint.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef size_t size_t;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_LIGHT_GREY 7

static inline u8 vga_entry_color(u8 fg, u8 bg) { return fg | bg << 4; }

static inline u16 vga_entry(unsigned char uc, u8 color) {
  return (u16)uc | (u16)color << 8;
}

static u16 mock_vga_buffer[VGA_WIDTH * VGA_HEIGHT];
static size_t terminal_row;
static size_t terminal_column;
static u8 terminal_color;
static u16 *terminal_buffer;

void mock_vga_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  terminal_buffer = mock_vga_buffer;
  memset(mock_vga_buffer, 0, sizeof(mock_vga_buffer));
}

void mock_vga_putentryat(char c, u8 color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = vga_entry(c, color);
}

void mock_vga_putchar(char c) {
  if (c == '\n') {
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT) {
      terminal_row = 0;
    }
    return;
  }

  mock_vga_putentryat(c, terminal_color, terminal_column, terminal_row);

  if (++terminal_column == VGA_WIDTH) {
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT) {
      terminal_row = 0;
    }
  }
}

void mock_vga_writestring(const char *data) {
  while (*data) {
    mock_vga_putchar(*data++);
  }
}

TEST(vga_initialize_sets_position_to_zero) {
  mock_vga_initialize();
  ASSERT_EQ(terminal_row, 0);
  ASSERT_EQ(terminal_column, 0);
  TEST_PASS_MSG();
}

TEST(vga_putchar_writes_to_correct_position) {
  mock_vga_initialize();
  mock_vga_putchar('A');
  u8 chr = mock_vga_buffer[0] & 0xFF;
  ASSERT_EQ(chr, 'A');
  TEST_PASS_MSG();
}

TEST(vga_putchar_advances_column) {
  mock_vga_initialize();
  mock_vga_putchar('A');
  ASSERT_EQ(terminal_column, 1);
  ASSERT_EQ(terminal_row, 0);
  TEST_PASS_MSG();
}

TEST(vga_putchar_wraps_at_end_of_line) {
  mock_vga_initialize();
  for (int i = 0; i < VGA_WIDTH; i++) {
    mock_vga_putchar('X');
  }
  ASSERT_EQ(terminal_column, 0);
  ASSERT_EQ(terminal_row, 1);
  TEST_PASS_MSG();
}

TEST(vga_putchar_handles_newline) {
  mock_vga_initialize();
  mock_vga_putchar('A');
  mock_vga_putchar('\n');
  ASSERT_EQ(terminal_column, 0);
  ASSERT_EQ(terminal_row, 1);
  TEST_PASS_MSG();
}

TEST(vga_writestring_writes_multiple_chars) {
  mock_vga_initialize();
  mock_vga_writestring("Hello");

  u8 h = mock_vga_buffer[0] & 0xFF;
  u8 e = mock_vga_buffer[1] & 0xFF;
  u8 l1 = mock_vga_buffer[2] & 0xFF;
  u8 l2 = mock_vga_buffer[3] & 0xFF;
  u8 o = mock_vga_buffer[4] & 0xFF;

  ASSERT_EQ(h, 'H');
  ASSERT_EQ(e, 'e');
  ASSERT_EQ(l1, 'l');
  ASSERT_EQ(l2, 'l');
  ASSERT_EQ(o, 'o');
  TEST_PASS_MSG();
}

TEST(vga_writestring_with_newline) {
  mock_vga_initialize();
  mock_vga_writestring("Line1\nLine2");

  u8 L1 = mock_vga_buffer[0] & 0xFF;
  u8 L2 = mock_vga_buffer[VGA_WIDTH] & 0xFF;

  ASSERT_EQ(L1, 'L');
  ASSERT_EQ(L2, 'L');
  TEST_PASS_MSG();
}

TEST(vga_color_preserved_in_buffer) {
  mock_vga_initialize();
  mock_vga_putchar('X');

  u8 color = (mock_vga_buffer[0] >> 8) & 0xFF;
  ASSERT_EQ(color, terminal_color);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nVGA Driver Tests:\n");

  RUN_TEST(vga_initialize_sets_position_to_zero);
  RUN_TEST(vga_putchar_writes_to_correct_position);
  RUN_TEST(vga_putchar_advances_column);
  RUN_TEST(vga_putchar_wraps_at_end_of_line);
  RUN_TEST(vga_putchar_handles_newline);
  RUN_TEST(vga_writestring_writes_multiple_chars);
  RUN_TEST(vga_writestring_with_newline);
  RUN_TEST(vga_color_preserved_in_buffer);

  TEST_SUMMARY();
}
