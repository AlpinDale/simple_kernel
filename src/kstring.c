#include "kstring.h"

void *kmemcpy(void *dst, const void *src, size_t n) {
  u8 *out = (u8 *)dst;
  const u8 *in = (const u8 *)src;

  for (size_t i = 0; i < n; i++) {
    out[i] = in[i];
  }

  return dst;
}

void *kmemmove(void *dst, const void *src, size_t n) {
  u8 *out = (u8 *)dst;
  const u8 *in = (const u8 *)src;

  if (out == in || n == 0) {
    return dst;
  }

  if (out < in) {
    return kmemcpy(dst, src, n);
  }

  for (size_t i = n; i > 0; i--) {
    out[i - 1] = in[i - 1];
  }

  return dst;
}

void *kmemset(void *dst, int value, size_t n) {
  u8 *out = (u8 *)dst;

  for (size_t i = 0; i < n; i++) {
    out[i] = (u8)value;
  }

  return dst;
}

size_t kstrlen(const char *str) {
  size_t len = 0;

  while (str[len] != '\0') {
    len++;
  }

  return len;
}

int kstrcmp(const char *lhs, const char *rhs) {
  while (*lhs != '\0' && *lhs == *rhs) {
    lhs++;
    rhs++;
  }

  return (unsigned char)*lhs - (unsigned char)*rhs;
}

int kstrncmp(const char *lhs, const char *rhs, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (lhs[i] != rhs[i] || lhs[i] == '\0' || rhs[i] == '\0') {
      return (unsigned char)lhs[i] - (unsigned char)rhs[i];
    }
  }

  return 0;
}

int ku64_parse(const char *str, u64 *value) {
  if (str == 0 || *str == '\0' || value == 0) {
    return 0;
  }

  u64 out = 0;
  while (*str != '\0') {
    if (*str < '0' || *str > '9') {
      return 0;
    }

    out = out * 10 + (u64)(*str - '0');
    str++;
  }

  *value = out;
  return 1;
}

static char *convert_u64(char *buffer, size_t size, u64 value, u32 base,
                         const char *digits) {
  char tmp[32];
  size_t pos = 0;

  if (size == 0) {
    return buffer;
  }

  if (value == 0) {
    if (size > 1) {
      buffer[0] = '0';
      buffer[1] = '\0';
    } else {
      buffer[0] = '\0';
    }
    return buffer;
  }

  while (value > 0 && pos < sizeof(tmp)) {
    tmp[pos++] = digits[value % base];
    value /= base;
  }

  size_t out = 0;
  while (pos > 0 && out + 1 < size) {
    buffer[out++] = tmp[--pos];
  }
  buffer[out] = '\0';

  return buffer;
}

char *ku64_to_dec(char *buffer, size_t size, u64 value) {
  return convert_u64(buffer, size, value, 10, "0123456789");
}

char *ku64_to_hex(char *buffer, size_t size, u64 value) {
  return convert_u64(buffer, size, value, 16, "0123456789ABCDEF");
}
