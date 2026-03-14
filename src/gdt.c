#include "gdt.h"
#include "kstring.h"
#include "types.h"

struct gdt64_entry {
  u16 limit_low;
  u16 base_low;
  u8 base_mid;
  u8 access;
  u8 granularity;
  u8 base_high;
} __attribute__((packed));

struct tss_entry {
  u32 reserved0;
  u64 rsp0;
  u64 rsp1;
  u64 rsp2;
  u64 reserved1;
  u64 ist1;
  u64 ist2;
  u64 ist3;
  u64 ist4;
  u64 ist5;
  u64 ist6;
  u64 ist7;
  u64 reserved2;
  u16 reserved3;
  u16 iomap_base;
} __attribute__((packed));

struct gdt_ptr {
  u16 limit;
  u64 base;
} __attribute__((packed));

struct tss_descriptor {
  u16 limit_low;
  u16 base_low;
  u8 base_mid;
  u8 access;
  u8 granularity;
  u8 base_high;
  u32 base_upper;
  u32 reserved;
} __attribute__((packed));

extern void gdt_load(const void *ptr);
extern void tss_load(u16 selector);

static struct {
  struct gdt64_entry null_seg;
  struct gdt64_entry code_seg;
  struct gdt64_entry data_seg;
  struct tss_descriptor tss_seg;
} __attribute__((packed)) gdt;

static struct tss_entry tss;

void gdt_init(void) {
  kmemset(&gdt, 0, sizeof(gdt));
  kmemset(&tss, 0, sizeof(tss));

  gdt.code_seg.access = 0x9A;
  gdt.code_seg.granularity = 0x20;
  gdt.data_seg.access = 0x92;

  uintptr_t base = (uintptr_t)&tss;
  u32 limit = sizeof(tss) - 1;

  gdt.tss_seg.limit_low = limit & 0xFFFF;
  gdt.tss_seg.base_low = base & 0xFFFF;
  gdt.tss_seg.base_mid = (base >> 16) & 0xFF;
  gdt.tss_seg.access = 0x89;
  gdt.tss_seg.granularity = (limit >> 16) & 0x0F;
  gdt.tss_seg.base_high = (base >> 24) & 0xFF;
  gdt.tss_seg.base_upper = (base >> 32) & 0xFFFFFFFFu;

  struct gdt_ptr ptr;
  ptr.limit = sizeof(gdt) - 1;
  ptr.base = (u64)(uintptr_t)&gdt;

  gdt_load(&ptr);
  tss_load(0x18);
}
