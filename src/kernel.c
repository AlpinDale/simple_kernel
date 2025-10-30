#include "idt.h"
#include "keyboard.h"
#include "vga.h"

void kernel_main(void) {
  volatile unsigned short *vga = (volatile unsigned short *)0xB8000;

  for (int i = 0; i < 80 * 25; i++) {
    vga[i] = 0x0F20; // space
  }

  vga_initialize();

  vga_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));

  vga_writestring("The worst kernel to ever exist!\n\n");
  vga_writestring("Press Enter to exit.");

  idt_init();
  keyboard_init();

  __asm__ volatile("sti");

  while (!keyboard_should_exit()) {
    __asm__ volatile("hlt");
  }

  vga_writestring("\n\nExiting kernel...");

  __asm__ volatile("outw %0, %1"
                   :
                   : "a"((unsigned short)0x2000), "Nd"((unsigned short)0x604));

  for (;;) {
    __asm__ volatile("cli; hlt");
  }
}
