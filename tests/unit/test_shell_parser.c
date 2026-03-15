#include "../test.h"

#include "../../src/boot_info.h"
#include "../../src/input.h"
#include "../../src/cpuinfo.h"
#include "../../src/pci.h"
#include "../../src/rtc.h"
#include "../../src/shell.h"

void cpu_info_read(cpu_info_t *info) {
  if (info != 0) {
    memset(info, 0, sizeof(*info));
    strcpy(info->vendor, "UnitTestCPU");
  }
}
void kprint(const char *str) { (void)str; }
void kprint_u64(unsigned long long value) { (void)value; }
void kprint_hex(unsigned long long value) { (void)value; }
void kprint_colored(const char *str, unsigned char fg, unsigned char bg) {
  (void)str;
  (void)fg;
  (void)bg;
}
void kprint_rainbow(const char *str) { (void)str; }
void panic(const char *message) {
  (void)message;
}
void system_reboot(void) {}
void system_halt(void) {}
void system_qemu_exit(unsigned short code) { (void)code; }
unsigned long long system_get_uptime_ms(void) { return 0; }
unsigned long long pmm_total_bytes(void) { return 0; }
unsigned long long pmm_free_bytes(void) { return 0; }
unsigned long long pmm_reserved_bytes(void) { return 0; }
unsigned long long heap_total_bytes(void) { return 0; }
unsigned long long heap_used_bytes(void) { return 0; }
unsigned long long heap_free_bytes(void) { return 0; }
void *kmalloc(unsigned long size) {
  (void)size;
  return (void *)1;
}
void *kcalloc(unsigned long count, unsigned long size) {
  (void)count;
  (void)size;
  return (void *)1;
}
void kfree(void *ptr) { (void)ptr; }
const memory_map_entry_t *boot_get_memory_map(unsigned int *count) {
  if (count != 0) {
    *count = 0;
  }
  return 0;
}
const char *boot_memory_type_name(unsigned int type) {
  (void)type;
  return "usable";
}
int rtc_read_time(rtc_time_t *time) {
  if (time != 0) {
    memset(time, 0, sizeof(*time));
    time->year = 2026;
  }
  return 1;
}
input_event_t input_getevent(void) {
  input_event_t event;
  event.type = INPUT_EVENT_CHAR;
  event.ch = '\n';
  return event;
}
char input_getchar(void) { return '\n'; }
void vga_clear(void) {}
void vga_setcolor(unsigned char color) { (void)color; }
unsigned char vga_entry_color(unsigned char fg, unsigned char bg) {
  return fg | (bg << 4);
}
void system_sleep_ms(unsigned long long delay_ms) { (void)delay_ms; }
void vga_scroll_view(int delta) { (void)delta; }
void vga_scroll_to_bottom(void) {}
u8 pci_enumerate(pci_device_t *devices, u8 max_devices) {
  (void)devices;
  (void)max_devices;
  return 0;
}
const char *pci_class_name(u8 class_code, u8 subclass) {
  (void)class_code;
  (void)subclass;
  return "Unknown device";
}

TEST(parse_command_with_multiple_args) {
  char input[] = "echo hello world";
  char *cmd = 0;
  char *args[16];
  int argc = 0;

  shell_parse_command(input, &cmd, args, &argc);

  ASSERT_EQ(strcmp(cmd, "echo"), 0);
  ASSERT_EQ(argc, 2);
  ASSERT_EQ(strcmp(args[0], "hello"), 0);
  ASSERT_EQ(strcmp(args[1], "world"), 0);
  TEST_PASS_MSG();
}

TEST(parse_command_trims_leading_spaces) {
  char input[] = "   help";
  char *cmd = 0;
  char *args[16];
  int argc = 0;

  shell_parse_command(input, &cmd, args, &argc);
  ASSERT_EQ(strcmp(cmd, "help"), 0);
  ASSERT_EQ(argc, 0);
  TEST_PASS_MSG();
}

TEST(parse_command_handles_empty_input) {
  char input[] = "";
  char *cmd = 0;
  char *args[16];
  int argc = 0;

  shell_parse_command(input, &cmd, args, &argc);

  ASSERT_EQ(cmd[0], '\0');
  ASSERT_EQ(argc, 0);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nShell Parser Tests:\n");
  RUN_TEST(parse_command_with_multiple_args);
  RUN_TEST(parse_command_trims_leading_spaces);
  RUN_TEST(parse_command_handles_empty_input);
  TEST_SUMMARY();
}
