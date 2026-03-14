#ifndef PANIC_H
#define PANIC_H

#include "types.h"

void panic(const char *message);
void panicf(const char *prefix, u64 value);

#endif
