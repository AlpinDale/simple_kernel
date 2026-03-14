#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_CYAN 3
#define VGA_COLOR_RED 4
#define VGA_COLOR_MAGENTA 5
#define VGA_COLOR_BROWN 6
#define VGA_COLOR_LIGHT_GREY 7
#define VGA_COLOR_DARK_GREY 8
#define VGA_COLOR_LIGHT_BLUE 9
#define VGA_COLOR_LIGHT_GREEN 10
#define VGA_COLOR_LIGHT_CYAN 11
#define VGA_COLOR_LIGHT_RED 12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW 14
#define VGA_COLOR_WHITE 15

static inline u8 vga_entry_color(u8 fg, u8 bg) { return fg | bg << 4; }

static inline u16 vga_entry(unsigned char uc, u8 color) {
  return (u16)uc | (u16)color << 8;
}
void vga_initialize(void);
void vga_setcolor(u8 color);
void vga_putentryat(char c, u8 color, size_t x, size_t y);
void vga_putchar(char c);
void vga_write(const char *data, size_t size);
void vga_writestring(const char *data);
void vga_clear(void);
void vga_bind_buffer(u16 *buffer);
size_t vga_get_row(void);
size_t vga_get_column(void);
u8 vga_get_color(void);

#endif // VGA_H
