#include "../test.h"

#include "../../src/keyboard.h"

u8 test_inb(u16 port) {
  (void)port;
  return 0;
}

void test_outb(u16 port, u8 value) {
  (void)port;
  (void)value;
}

void test_outw(u16 port, u16 value) {
  (void)port;
  (void)value;
}

TEST(keyboard_translates_shifted_letters) {
  keyboard_reset_state();
  keyboard_process_scancode(0x2A);
  keyboard_process_scancode(0x1E);
  ASSERT_TRUE(keyboard_has_input());
  ASSERT_EQ(keyboard_getchar(), 'A');
  TEST_PASS_MSG();
}

TEST(keyboard_ignores_key_releases) {
  keyboard_reset_state();
  keyboard_process_scancode(0x9E);
  ASSERT_FALSE(keyboard_has_input());
  TEST_PASS_MSG();
}

TEST(keyboard_ring_buffer_wraps) {
  keyboard_reset_state();
  for (int i = 0; i < INPUT_BUFFER_SIZE - 1; i++) {
    keyboard_process_scancode(0x1E);
  }

  for (int i = 0; i < INPUT_BUFFER_SIZE - 2; i++) {
    ASSERT_EQ(keyboard_getchar(), 'a');
  }

  TEST_PASS_MSG();
}

int main(void) {
  printf("\nKeyboard Tests:\n");
  RUN_TEST(keyboard_translates_shifted_letters);
  RUN_TEST(keyboard_ignores_key_releases);
  RUN_TEST(keyboard_ring_buffer_wraps);
  TEST_SUMMARY();
}
