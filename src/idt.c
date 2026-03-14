#include "idt.h"

#define IDT_ENTRIES 256

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

extern void idt_load(u64 idt_ptr);

void idt_set_gate(u8 num, u64 handler, u16 selector, u8 flags) {
  idt[num].offset_low = handler & 0xFFFF;
  idt[num].selector = selector;
  idt[num].ist = 0;
  idt[num].type_attr = flags;
  idt[num].offset_mid = (handler >> 16) & 0xFFFF;
  idt[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
  idt[num].zero = 0;
}

void idt_init(void) {
  idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
  idtp.base = (u64)&idt;

  for (int i = 0; i < IDT_ENTRIES; i++) {
    idt[i].offset_low = 0;
    idt[i].selector = 0;
    idt[i].ist = 0;
    idt[i].type_attr = 0;
    idt[i].offset_mid = 0;
    idt[i].offset_high = 0;
    idt[i].zero = 0;
  }

}

void idt_load_current(void) { idt_load((u64)&idtp); }
