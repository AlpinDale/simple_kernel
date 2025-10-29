#include "../test.h"
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;

#define ENTER_SCANCODE 0x1C
#define ESC_SCANCODE 0x01
#define A_SCANCODE 0x1E

TEST(enter_scancode_value) {
  ASSERT_EQ(ENTER_SCANCODE, 0x1C);
  TEST_PASS_MSG();
}

TEST(scancode_is_enter) {
  u8 scancode = 0x1C;
  ASSERT_TRUE(scancode == ENTER_SCANCODE);
  TEST_PASS_MSG();
}

TEST(scancode_is_not_enter) {
  u8 scancode = 0x1E;
  ASSERT_FALSE(scancode == ENTER_SCANCODE);
  TEST_PASS_MSG();
}

TEST(pic_eoi_port) {
  u16 pic_command_port = 0x20;
  u8 eoi_command = 0x20;
  ASSERT_EQ(pic_command_port, 0x20);
  ASSERT_EQ(eoi_command, 0x20);
  TEST_PASS_MSG();
}

TEST(keyboard_data_port) {
  u16 keyboard_port = 0x60;
  ASSERT_EQ(keyboard_port, 0x60);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nKeyboard Tests:\n");

  RUN_TEST(enter_scancode_value);
  RUN_TEST(scancode_is_enter);
  RUN_TEST(scancode_is_not_enter);
  RUN_TEST(pic_eoi_port);
  RUN_TEST(keyboard_data_port);

  TEST_SUMMARY();
}
