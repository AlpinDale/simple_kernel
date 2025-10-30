#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

#define INPUT_BUFFER_SIZE 256

typedef struct {
  char buffer[INPUT_BUFFER_SIZE];
  u16 length;
  u8 shift_pressed;
} keyboard_state_t;

void keyboard_init(void);
void keyboard_handler(void);
char keyboard_getchar(void);
u8 keyboard_has_input(void);
void keyboard_get_line(char *buffer, u16 max_len);

#endif
