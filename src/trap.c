#include "trap.h"

#include "cpu.h"
#include "idt.h"
#include "io.h"
#include "keyboard.h"
#include "kprint.h"
#include "panic.h"
#include "system.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

extern void *isr_stub_table[];

static const char *const exception_names[] = {
    "Divide Error",           "Debug",               "NMI",
    "Breakpoint",             "Overflow",            "Bound Range",
    "Invalid Opcode",         "Device Not Available","Double Fault",
    "Coprocessor Segment",    "Invalid TSS",         "Segment Not Present",
    "Stack Fault",            "General Protection",  "Page Fault",
    "Reserved",               "x87 Floating Point",  "Alignment Check",
    "Machine Check",          "SIMD Floating Point", "Virtualization",
    "Control Protection",     "Reserved",            "Reserved",
    "Reserved",               "Reserved",            "Reserved",
    "Reserved",               "Hypervisor Injection","VMM Communication",
    "Security",               "Reserved",
};

static void pic_remap(void) {
  outb(PIC1_COMMAND, 0x11);
  outb(PIC2_COMMAND, 0x11);
  outb(PIC1_DATA, 0x20);
  outb(PIC2_DATA, 0x28);
  outb(PIC1_DATA, 0x04);
  outb(PIC2_DATA, 0x02);
  outb(PIC1_DATA, 0x01);
  outb(PIC2_DATA, 0x01);
  outb(PIC1_DATA, 0xF8);
  outb(PIC2_DATA, 0xFF);
}

static void pic_send_eoi(u8 irq) {
  if (irq >= 8) {
    outb(PIC2_COMMAND, 0x20);
  }
  outb(PIC1_COMMAND, 0x20);
}

void trap_init(void) {
  idt_init();
  pic_remap();

  for (u8 vector = 0; vector < 48; vector++) {
    idt_set_gate(vector, (u64)(uintptr_t)isr_stub_table[vector], 0x08, 0x8E);
  }

  idt_load_current();
}

static void dump_frame(trap_frame_t *frame) {
  kprint(" vector=");
  kprint_u64(frame->vector);
  kprint(" err=");
  kprint_hex(frame->error_code);
  kprint(" rip=");
  kprint_hex(frame->rip);
  kprint(" cs=");
  kprint_hex(frame->cs);
  kprint(" rflags=");
  kprint_hex(frame->rflags);
  if (frame->vector == 14) {
    kprint(" cr2=");
    kprint_hex(cpu_read_cr2());
  }
  kprint("\n rax=");
  kprint_hex(frame->rax);
  kprint(" rbx=");
  kprint_hex(frame->rbx);
  kprint(" rcx=");
  kprint_hex(frame->rcx);
  kprint(" rdx=");
  kprint_hex(frame->rdx);
  kprint("\n rsi=");
  kprint_hex(frame->rsi);
  kprint(" rdi=");
  kprint_hex(frame->rdi);
  kprint(" rbp=");
  kprint_hex(frame->rbp);
  kprint("\n r8=");
  kprint_hex(frame->r8);
  kprint(" r9=");
  kprint_hex(frame->r9);
  kprint(" r10=");
  kprint_hex(frame->r10);
  kprint(" r11=");
  kprint_hex(frame->r11);
  kprint("\n r12=");
  kprint_hex(frame->r12);
  kprint(" r13=");
  kprint_hex(frame->r13);
  kprint(" r14=");
  kprint_hex(frame->r14);
  kprint(" r15=");
  kprint_hex(frame->r15);
  kprint("\n");
}

void trap_dispatch(trap_frame_t *frame) {
  if (frame->vector < 32) {
    kprint_colored("\nEXCEPTION: ", VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    kprint(exception_names[frame->vector]);
    dump_frame(frame);
    system_qemu_exit(0x22);
    cpu_cli();
    for (;;) {
      cpu_hlt();
    }
  }

  switch (frame->vector) {
  case 32:
    system_timer_tick();
    pic_send_eoi(0);
    return;
  case 33:
    keyboard_handler();
    pic_send_eoi(1);
    return;
  case 39:
    pic_send_eoi(7);
    return;
  default:
    panicf("Unhandled IRQ vector ", frame->vector);
  }
}
