#include "kprint.h"

void kprint(const char *str) { vga_writestring(str); }

void kprint_colored(const char *str, u8 fg, u8 bg) {
  vga_setcolor(vga_entry_color(fg, bg));
  vga_writestring(str);
  vga_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}

void kprint_rainbow(const char *str) {
  u8 color_index = 1;

  while (*str) {
    if (*str == '\n') {
      vga_putchar('\n');
      str++;
      continue;
    }

    // Cycle through colors 1-15 (skip 0 which is black)
    vga_setcolor(vga_entry_color(color_index, VGA_COLOR_BLACK));

    char temp[2] = {*str, '\0'};
    vga_writestring(temp);

    color_index++;
    if (color_index > 15) {
      color_index = 1;
    }

    str++;
  }

  // Reset to default color
  vga_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
}
