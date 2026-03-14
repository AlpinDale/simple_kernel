#include "keyboard.h"
#include "cpu.h"
#include "io.h"
#include "keyboard_map.h"

#define KEYBOARD_DATA_PORT 0x60

static keyboard_state_t kbd_state = {0};
static char input_queue[INPUT_BUFFER_SIZE];
static u16 input_queue_head = 0;
static u16 input_queue_tail = 0;

static void queue_char(char c) {
  u16 next = (input_queue_head + 1) % INPUT_BUFFER_SIZE;
  if (next != input_queue_tail) {
    input_queue[input_queue_head] = c;
    input_queue_head = next;
  }
}

void keyboard_process_scancode(u8 scancode) {
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
    queue_char(ascii);
  }
}

void keyboard_handler(void) { keyboard_process_scancode(inb(KEYBOARD_DATA_PORT)); }

u8 keyboard_has_input(void) { return input_queue_head != input_queue_tail; }

char keyboard_getchar(void) {
  while (!keyboard_has_input()) {
    cpu_hlt();
  }

  char c = input_queue[input_queue_tail];
  input_queue_tail = (input_queue_tail + 1) % INPUT_BUFFER_SIZE;
  return c;
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
  input_queue_head = 0;
  input_queue_tail = 0;
}

void keyboard_init(void) { keyboard_reset_state(); }
