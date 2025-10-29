#include "idt.h"
#include "keyboard.h"

static void write_string(volatile unsigned short *vga, int row,
                         const char *msg) {
  int offset = row * 80;
  for (int i = 0; msg[i] != '\0'; i++) {
    vga[offset + i] = 0x0F00 | msg[i];
  }
}

void kernel_main(void) {
  volatile unsigned short *vga = (volatile unsigned short *)0xB8000;

  for (int i = 0; i < 80 * 25; i++) {
    vga[i] = 0x0F00;
  }

  write_string(vga, 0, "The worst kernel to ever exist!");
  write_string(vga, 2, "Press Enter to exit.");

  idt_init();
  keyboard_init();

  __asm__ volatile("sti");

  while (!keyboard_should_exit()) {
    __asm__ volatile("hlt");
  }

  write_string(vga, 4, "Exiting kernel...");

  __asm__ volatile("outw %0, %1"
                   :
                   : "a"((unsigned short)0x2000), "Nd"((unsigned short)0x604));

  for (;;) {
    __asm__ volatile("cli; hlt");
  }
}
