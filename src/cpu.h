#ifndef CPU_H
#define CPU_H

#ifdef KERNEL_TEST
void test_cpuid(unsigned int leaf, unsigned int subleaf, unsigned int *eax,
                unsigned int *ebx, unsigned int *ecx, unsigned int *edx);
static inline void cpu_hlt(void) {}
static inline void cpu_cli(void) {}
static inline void cpu_sti(void) {}
static inline unsigned long long cpu_read_cr2(void) { return 0; }
static inline void cpu_cpuid(unsigned int leaf, unsigned int subleaf,
                             unsigned int *eax, unsigned int *ebx,
                             unsigned int *ecx, unsigned int *edx) {
  test_cpuid(leaf, subleaf, eax, ebx, ecx, edx);
}
#else
static inline void cpu_hlt(void) { __asm__ volatile("hlt"); }
static inline void cpu_cli(void) { __asm__ volatile("cli"); }
static inline void cpu_sti(void) { __asm__ volatile("sti"); }
static inline unsigned long long cpu_read_cr2(void) {
  unsigned long long value;
  __asm__ volatile("mov %%cr2, %0" : "=r"(value));
  return value;
}
static inline void cpu_cpuid(unsigned int leaf, unsigned int subleaf,
                             unsigned int *eax, unsigned int *ebx,
                             unsigned int *ecx, unsigned int *edx) {
  unsigned int out_eax;
  unsigned int out_ebx;
  unsigned int out_ecx;
  unsigned int out_edx;

  __asm__ volatile("cpuid"
                   : "=a"(out_eax), "=b"(out_ebx), "=c"(out_ecx), "=d"(out_edx)
                   : "a"(leaf), "c"(subleaf));

  if (eax != 0) {
    *eax = out_eax;
  }
  if (ebx != 0) {
    *ebx = out_ebx;
  }
  if (ecx != 0) {
    *ecx = out_ecx;
  }
  if (edx != 0) {
    *edx = out_edx;
  }
}
#endif

#endif
