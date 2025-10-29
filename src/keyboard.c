#include "keyboard.h"
#include "idt.h"

static bool exit_requested = false;

extern void keyboard_handler_asm(void);

static inline u8 inb(u16 port) {
  u8 ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void outb(u16 port, u8 val) {
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void keyboard_init(void) {
  outb(0x20, 0x11);
  outb(0xA0, 0x11);
  outb(0x21, 0x20);
  outb(0xA1, 0x28);
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);
  outb(0x21, 0xFD);
  outb(0xA1, 0xFF);

  idt_set_gate(33, (u64)keyboard_handler_asm, 0x08, 0x8E);
  exit_requested = false;
}

void keyboard_handler(void) {
  u8 scancode = inb(KEYBOARD_DATA_PORT);

  if (scancode == ENTER_SCANCODE) {
    exit_requested = true;
  }

  outb(0x20, 0x20);
}

bool keyboard_should_exit(void) { return exit_requested; }
