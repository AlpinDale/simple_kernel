#include "input.h"
#include "cpu.h"
#include "keyboard.h"
#include "serial.h"

char input_getchar(void) {
  for (;;) {
    if (serial_has_input()) {
      char c = serial_read_char();
      if (c == '\r') {
        c = '\n';
      }
      return c;
    }

    if (keyboard_has_input()) {
      return keyboard_getchar();
    }

    cpu_hlt();
  }
}
