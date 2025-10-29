#ifndef IDT_H
#define IDT_H

#include "types.h"

struct idt_entry {
  u16 offset_low;
  u16 selector;
  u8 ist;
  u8 type_attr;
  u16 offset_mid;
  u32 offset_high;
  u32 zero;
} __attribute__((packed));

struct idt_ptr {
  u16 limit;
  u64 base;
} __attribute__((packed));

void idt_init(void);
void idt_set_gate(u8 num, u64 handler, u16 selector, u8 flags);

#endif // IDT_H
