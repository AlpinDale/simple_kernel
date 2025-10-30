#ifndef KPRINT_H
#define KPRINT_H

#include "vga.h"

void kprint(const char *str);
void kprint_colored(const char *str, u8 fg, u8 bg);
void kprint_rainbow(const char *str);

#endif
