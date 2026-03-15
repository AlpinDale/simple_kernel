#ifndef IO_H
#define IO_H

#include "types.h"

#ifdef KERNEL_TEST
u8 test_inb(u16 port);
void test_outb(u16 port, u8 value);
void test_outw(u16 port, u16 value);
u32 test_inl(u16 port);
void test_outl(u16 port, u32 value);

static inline u8 inb(u16 port) { return test_inb(port); }
static inline void outb(u16 port, u8 value) { test_outb(port, value); }
static inline void outw(u16 port, u16 value) { test_outw(port, value); }
static inline u32 inl(u16 port) { return test_inl(port); }
static inline void outl(u16 port, u32 value) { test_outl(port, value); }
#else
static inline u8 inb(u16 port) {
  u8 ret;
  __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void outb(u16 port, u8 value) {
  __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(u16 port, u16 value) {
  __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline u32 inl(u16 port) {
  u32 ret;
  __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void outl(u16 port, u32 value) {
  __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}
#endif

#endif
