#ifndef KSTRING_H
#define KSTRING_H

#include "types.h"

void *kmemcpy(void *dst, const void *src, size_t n);
void *kmemmove(void *dst, const void *src, size_t n);
void *kmemset(void *dst, int value, size_t n);
size_t kstrlen(const char *str);
int kstrcmp(const char *lhs, const char *rhs);
int kstrncmp(const char *lhs, const char *rhs, size_t n);
char *ku64_to_dec(char *buffer, size_t size, u64 value);
char *ku64_to_hex(char *buffer, size_t size, u64 value);

#endif
