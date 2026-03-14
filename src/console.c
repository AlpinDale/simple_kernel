#include "console.h"
#include "serial.h"
#include "vga.h"

void console_init(void) {
  vga_initialize();
  serial_init();
}

void console_write_char(char c) {
  vga_putchar(c);
  serial_write_char(c);
}

void console_write(const char *str) {
  while (*str != '\0') {
    console_write_char(*str++);
  }
}

void console_setcolor(u8 color) { vga_setcolor(color); }
