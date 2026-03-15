#include "keyboard.h"
#include "cpu.h"
#include "io.h"
#include "keyboard_map.h"

#define KEYBOARD_DATA_PORT 0x60

static keyboard_state_t kbd_state = {0};
static input_event_t input_queue[INPUT_BUFFER_SIZE];
static u16 input_queue_head = 0;
static u16 input_queue_tail = 0;

static input_event_t keyboard_event_char(char c) {
  input_event_t event;
  event.type = INPUT_EVENT_CHAR;
  event.ch = c;
  return event;
}

static input_event_t keyboard_event_special(input_event_type_t type) {
  input_event_t event;
  event.type = type;
  event.ch = '\0';
  return event;
}

static void queue_event(input_event_t event) {
  u16 next = (input_queue_head + 1) % INPUT_BUFFER_SIZE;
  if (next != input_queue_tail) {
    input_queue[input_queue_head] = event;
    input_queue_head = next;
  }
}

void keyboard_process_scancode(u8 scancode) {
  if (scancode == KEY_EXTENDED_PREFIX) {
    kbd_state.extended_prefix = 1;
    return;
  }

  if (kbd_state.extended_prefix) {
    kbd_state.extended_prefix = 0;

    if (scancode & 0x80) {
      return;
    }

    switch (scancode) {
    case KEY_ARROW_UP:
      queue_event(keyboard_event_special(INPUT_EVENT_UP));
      return;
    case KEY_ARROW_DOWN:
      queue_event(keyboard_event_special(INPUT_EVENT_DOWN));
      return;
    case KEY_ARROW_LEFT:
      queue_event(keyboard_event_special(INPUT_EVENT_LEFT));
      return;
    case KEY_ARROW_RIGHT:
      queue_event(keyboard_event_special(INPUT_EVENT_RIGHT));
      return;
    default:
      return;
    }
  }

  if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT) {
    kbd_state.shift_pressed = 1;
    return;
  }
  if (scancode == KEY_LSHIFT_RELEASE || scancode == KEY_RSHIFT_RELEASE) {
    kbd_state.shift_pressed = 0;
    return;
  }

  if (scancode & 0x80) {
    return;
  }

  char ascii = 0;
  if (scancode < sizeof(scancode_to_ascii)) {
    if (kbd_state.shift_pressed) {
      ascii = scancode_to_ascii_shift[scancode];
    } else {
      ascii = scancode_to_ascii[scancode];
    }
  }

  if (ascii != 0) {
    queue_event(keyboard_event_char(ascii));
  }
}

void keyboard_handler(void) { keyboard_process_scancode(inb(KEYBOARD_DATA_PORT)); }

u8 keyboard_has_input(void) { return input_queue_head != input_queue_tail; }

input_event_t keyboard_getevent(void) {
  while (!keyboard_has_input()) {
    cpu_hlt();
  }

  input_event_t event = input_queue[input_queue_tail];
  input_queue_tail = (input_queue_tail + 1) % INPUT_BUFFER_SIZE;
  return event;
}

char keyboard_getchar(void) {
  for (;;) {
    input_event_t event = keyboard_getevent();
    if (event.type == INPUT_EVENT_CHAR) {
      return event.ch;
    }
  }
}

void keyboard_get_line(char *buffer, u16 max_len) {
  u16 pos = 0;
  while (pos < max_len - 1) {
    char c = keyboard_getchar();

    if (c == '\n') {
      buffer[pos] = '\0';
      return;
    } else if (c == '\b' && pos > 0) {
      pos--;
    } else if (c != '\b') {
      buffer[pos++] = c;
    }
  }
  buffer[pos] = '\0';
}

void keyboard_reset_state(void) {
  kbd_state.shift_pressed = 0;
  kbd_state.extended_prefix = 0;
  input_queue_head = 0;
  input_queue_tail = 0;
}

void keyboard_init(void) { keyboard_reset_state(); }
