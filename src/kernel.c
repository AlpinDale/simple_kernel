#include "idt.h"
#include "keyboard.h"
#include "shell.h"
#include "vga.h"

void kernel_main(void) {
  vga_initialize();
  idt_init();
  keyboard_init();

  __asm__ volatile("sti");

  shell_run();

  __asm__ volatile("outw %0, %1"
                   :
                   : "a"((unsigned short)0x2000), "Nd"((unsigned short)0x604));

  for (;;) {
    __asm__ volatile("cli; hlt");
  }
}
