#include "vga.h"

static size_t terminal_row;
static size_t terminal_column;
static u8 terminal_color;
static u16 *terminal_buffer;

static inline void outb(u16 port, u8 value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void update_cursor(void) {
  u16 pos = terminal_row * VGA_WIDTH + terminal_column;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (u8)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

void vga_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  terminal_buffer = (u16 *)VGA_MEMORY;
  update_cursor();
}

void vga_setcolor(u8 color) { terminal_color = color; }

void vga_putentryat(char c, u8 color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = vga_entry(c, color);
}

void vga_putchar(char c) {
  if (c == '\n') {
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT) {
      terminal_row = 0;
    }
    update_cursor();
    return;
  }

  if (c == '\b') {
    if (terminal_column > 0) {
      terminal_column--;
      vga_putentryat(' ', terminal_color, terminal_column, terminal_row);
    }
    update_cursor();
    return;
  }

  vga_putentryat(c, terminal_color, terminal_column, terminal_row);

  if (++terminal_column == VGA_WIDTH) {
    terminal_column = 0;
    if (++terminal_row == VGA_HEIGHT) {
      terminal_row = 0;
    }
  }

  update_cursor();
}

void vga_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    vga_putchar(data[i]);
  }
}

void vga_writestring(const char *data) {
  size_t len = 0;
  while (data[len]) {
    len++;
  }
  vga_write(data, len);
}

void vga_clear(void) {
  u16 blank = vga_entry(' ', terminal_color);

  __asm__ volatile("cld\n\t"
                   "rep stosw"
                   : "=D"(terminal_buffer), "=c"(blank)
                   : "D"(terminal_buffer), "a"(blank),
                     "c"(VGA_WIDTH * VGA_HEIGHT)
                   : "memory");

  terminal_row = 0;
  terminal_column = 0;
  update_cursor();
}
