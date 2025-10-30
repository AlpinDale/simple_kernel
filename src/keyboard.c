#include "keyboard.h"
#include "idt.h"
#include "keyboard_map.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define KEYBOARD_DATA_PORT 0x60

static keyboard_state_t kbd_state = {0};
static char input_queue[INPUT_BUFFER_SIZE];
static u16 input_queue_head = 0;
static u16 input_queue_tail = 0;

static inline void outb(u16 port, u8 value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {
  u8 ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static void queue_char(char c) {
  u16 next = (input_queue_head + 1) % INPUT_BUFFER_SIZE;
  if (next != input_queue_tail) {
    input_queue[input_queue_head] = c;
    input_queue_head = next;
  }
}

void keyboard_handler(void) {
  u8 scancode = inb(KEYBOARD_DATA_PORT);

  if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT) {
    kbd_state.shift_pressed = 1;
    outb(PIC1_COMMAND, 0x20);
    return;
  }
  if (scancode == KEY_LSHIFT_RELEASE || scancode == KEY_RSHIFT_RELEASE) {
    kbd_state.shift_pressed = 0;
    outb(PIC1_COMMAND, 0x20);
    return;
  }

  // ignore key releases (scancodes >= 0x80)
  if (scancode & 0x80) {
    outb(PIC1_COMMAND, 0x20);
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

  outb(PIC1_COMMAND, 0x20);
}

u8 keyboard_has_input(void) { return input_queue_head != input_queue_tail; }

char keyboard_getchar(void) {
  while (!keyboard_has_input()) {
    __asm__ volatile("hlt");
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

void keyboard_init(void) {
  outb(PIC1_COMMAND, 0x11);
  outb(PIC2_COMMAND, 0x11);
  outb(PIC1_DATA, 0x20);
  outb(PIC2_DATA, 0x28);
  outb(PIC1_DATA, 0x04);
  outb(PIC2_DATA, 0x02);
  outb(PIC1_DATA, 0x01);
  outb(PIC2_DATA, 0x01);
  outb(PIC1_DATA, 0xFC); // IRQ0 (timer) and IRQ1 (keyboard)
  outb(PIC2_DATA, 0xFF);

  extern void keyboard_handler_asm(void);
  idt_set_gate(33, (u64)keyboard_handler_asm, 0x08, 0x8E);
}
