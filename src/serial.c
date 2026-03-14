#include "serial.h"
#include "io.h"

#define COM1_PORT 0x3F8

static u8 serial_initialized = 0;

static u8 serial_tx_ready(void) { return (inb(COM1_PORT + 5) & 0x20) != 0; }

void serial_init(void) {
  outb(COM1_PORT + 1, 0x00);
  outb(COM1_PORT + 3, 0x80);
  outb(COM1_PORT + 0, 0x01);
  outb(COM1_PORT + 1, 0x00);
  outb(COM1_PORT + 3, 0x03);
  outb(COM1_PORT + 2, 0xC7);
  outb(COM1_PORT + 4, 0x0B);
  serial_initialized = 1;
}

void serial_write_char(char c) {
  if (!serial_initialized) {
    return;
  }

  if (c == '\n') {
    serial_write_char('\r');
  }

  while (!serial_tx_ready()) {
  }

  outb(COM1_PORT, (u8)c);
}

void serial_write(const char *str) {
  while (*str != '\0') {
    serial_write_char(*str++);
  }
}

u8 serial_has_input(void) {
  if (!serial_initialized) {
    return 0;
  }

  return (inb(COM1_PORT + 5) & 0x01) != 0;
}

char serial_read_char(void) {
  while (!serial_has_input()) {
  }

  return (char)inb(COM1_PORT);
}
