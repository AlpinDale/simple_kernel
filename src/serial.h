#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

void serial_init(void);
void serial_write_char(char c);
void serial_write(const char *str);
u8 serial_has_input(void);
char serial_read_char(void);

#endif
