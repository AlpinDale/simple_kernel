#include "../test.h"

#include "../../src/shell.h"

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
char input_getchar(void) { return '\n'; }
void vga_clear(void) {}
void vga_setcolor(unsigned char color) { (void)color; }
unsigned char vga_entry_color(unsigned char fg, unsigned char bg) {
  return fg | (bg << 4);
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

int main(void) {
  printf("\nShell Parser Tests:\n");
  RUN_TEST(parse_command_with_multiple_args);
  RUN_TEST(parse_command_trims_leading_spaces);
  TEST_SUMMARY();
}
