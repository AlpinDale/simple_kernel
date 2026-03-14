#include "console.h"
#include "kprint.h"
#include "kstring.h"

void kprint(const char *str) { console_write(str); }

void kprint_colored(const char *str, u8 fg, u8 bg) {
  console_setcolor(vga_entry_color(fg, bg));
  console_write(str);
  console_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

void kprint_rainbow(const char *str) {
  u8 color_index = 1;

  while (*str) {
    if (*str == '\n') {
      console_write_char('\n');
      str++;
      continue;
    }

    console_setcolor(vga_entry_color(color_index, VGA_COLOR_BLACK));

    console_write_char(*str);

    color_index++;
    if (color_index > 15) {
      color_index = 1;
    }

    str++;
  }

  console_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

void kprint_u64(u64 value) {
  char buffer[32];
  kprint(ku64_to_dec(buffer, sizeof(buffer), value));
}

void kprint_hex(u64 value) {
  char buffer[32];
  kprint("0x");
  kprint(ku64_to_hex(buffer, sizeof(buffer), value));
}
