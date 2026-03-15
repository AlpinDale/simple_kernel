#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "input.h"
#include "types.h"

#define INPUT_BUFFER_SIZE 256

typedef struct {
  u8 shift_pressed;
  u8 extended_prefix;
} keyboard_state_t;

void keyboard_init(void);
void keyboard_handler(void);
void keyboard_process_scancode(u8 scancode);
input_event_t keyboard_getevent(void);
char keyboard_getchar(void);
u8 keyboard_has_input(void);
void keyboard_get_line(char *buffer, u16 max_len);
void keyboard_reset_state(void);

#endif
