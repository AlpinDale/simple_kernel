#include "vga.h"
#include "io.h"
#include "kstring.h"

static size_t terminal_row;
static size_t terminal_column;
static u8 terminal_color;
static u16 *terminal_buffer;

static void update_cursor(void) {
  u16 pos = terminal_row * VGA_WIDTH + terminal_column;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (u8)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

static void vga_scroll(void) {
  if (terminal_row < VGA_HEIGHT) {
    return;
  }

  kmemmove(terminal_buffer, terminal_buffer + VGA_WIDTH,
           (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(u16));

  u16 blank = vga_entry(' ', terminal_color);
  for (size_t i = 0; i < VGA_WIDTH; i++) {
    terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + i] = blank;
  }

  terminal_row = VGA_HEIGHT - 1;
}

void vga_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  if (terminal_buffer == 0) {
    terminal_buffer = (u16 *)VGA_MEMORY;
  }
  vga_clear();
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
    terminal_row++;
    vga_scroll();
    update_cursor();
    return;
  }

  if (c == '\b') {
    if (terminal_column > 0 || terminal_row > 0) {
      if (terminal_column == 0) {
        terminal_row--;
        terminal_column = VGA_WIDTH;
      }
      terminal_column--;
      vga_putentryat(' ', terminal_color, terminal_column, terminal_row);
    }
    update_cursor();
    return;
  }

  vga_putentryat(c, terminal_color, terminal_column, terminal_row);

  if (++terminal_column == VGA_WIDTH) {
    terminal_column = 0;
    terminal_row++;
    vga_scroll();
  }

  update_cursor();
}

void vga_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    vga_putchar(data[i]);
  }
}

void vga_writestring(const char *data) {
  vga_write(data, kstrlen(data));
}

void vga_clear(void) {
  u16 blank = vga_entry(' ', terminal_color);
  for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
    terminal_buffer[i] = blank;
  }

  terminal_row = 0;
  terminal_column = 0;
  update_cursor();
}

void vga_bind_buffer(u16 *buffer) { terminal_buffer = buffer; }

size_t vga_get_row(void) { return terminal_row; }

size_t vga_get_column(void) { return terminal_column; }

u8 vga_get_color(void) { return terminal_color; }
