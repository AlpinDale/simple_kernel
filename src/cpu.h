#ifndef CPU_H
#define CPU_H

#ifdef KERNEL_TEST
static inline void cpu_hlt(void) {}
static inline void cpu_cli(void) {}
static inline void cpu_sti(void) {}
#else
static inline void cpu_hlt(void) { __asm__ volatile("hlt"); }
static inline void cpu_cli(void) { __asm__ volatile("cli"); }
static inline void cpu_sti(void) { __asm__ volatile("sti"); }
#endif

#endif
