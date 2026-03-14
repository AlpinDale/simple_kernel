#include "boot_info.h"
#include "console.h"
#include "cpu.h"
#include "gdt.h"
#include "keyboard.h"
#include "kprint.h"
#include "panic.h"
#include "pmm.h"
#include "shell.h"
#include "system.h"
#include "trap.h"
void kernel_main(const boot_info_t *boot_info) {
  console_init();

  if (boot_info == 0 || boot_info->magic != BOOT_INFO_MAGIC) {
    panic("missing boot info");
  }

  gdt_init();
  trap_init();
  keyboard_init();
  system_init_timer();
  pmm_init(boot_info);

  kprint("simple_kernel ");
  kprint(KERNEL_VERSION);
  kprint("\nboot_info memory entries: ");
  kprint_u64(boot_info->memory_map_entries);
  kprint("\nkernel phys start: ");
  kprint_hex(boot_info->kernel_phys_start);
  kprint("\nkernel phys end: ");
  kprint_hex(boot_info->kernel_phys_end);
  kprint("\n\n");

  cpu_sti();
  shell_run();
  system_qemu_exit(0x10);

  for (;;) {
    cpu_cli();
    cpu_hlt();
  }
}
