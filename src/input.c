#include "input.h"
#include "cpu.h"
#include "keyboard.h"
#include "serial.h"

static input_event_t input_event_char(char c) {
  input_event_t event;
  event.type = INPUT_EVENT_CHAR;
  event.ch = c;
  return event;
}

static input_event_t input_event_special(input_event_type_t type) {
  input_event_t event;
  event.type = type;
  event.ch = '\0';
  return event;
}

static input_event_t input_read_serial_event(void) {
  char c = serial_read_char();
  if (c == '\r') {
    c = '\n';
  }

  if (c != 0x1B) {
    return input_event_char(c);
  }

  if (!serial_has_input()) {
    return input_event_char(c);
  }

  if (serial_read_char() != '[') {
    return input_event_char(c);
  }

  switch (serial_read_char()) {
  case 'A':
    return input_event_special(INPUT_EVENT_UP);
  case 'B':
    return input_event_special(INPUT_EVENT_DOWN);
  case 'C':
    return input_event_special(INPUT_EVENT_RIGHT);
  case 'D':
    return input_event_special(INPUT_EVENT_LEFT);
  default:
    return input_event_char(c);
  }
}

input_event_t input_getevent(void) {
  for (;;) {
    if (serial_has_input()) {
      return input_read_serial_event();
    }

    if (keyboard_has_input()) {
      return keyboard_getevent();
    }

    cpu_hlt();
  }
}

char input_getchar(void) {
  for (;;) {
    input_event_t event = input_getevent();
    if (event.type == INPUT_EVENT_CHAR) {
      return event.ch;
    }
  }
}
