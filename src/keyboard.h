#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define ENTER_SCANCODE 0x1C

void keyboard_init(void);
void keyboard_handler(void);
bool keyboard_should_exit(void);

#endif // KEYBOARD_H
