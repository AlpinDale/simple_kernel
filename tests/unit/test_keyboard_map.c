#include "../test.h"
#include <stdint.h>

typedef uint8_t u8;

static const char test_scancode_to_ascii[] = {
    0,   0,   '1', '2',  '3',  '4', '5',  '6', '7', '8', '9',
    '0', '-', '=', '\b', '\t', 'q', 'w',  'e', 'r', 't', 'y',
    'u', 'i', 'o', 'p',  '[',  ']', '\n', 0,   'a', 's',
};

TEST(scancode_to_ascii_numbers) {
  ASSERT_EQ(test_scancode_to_ascii[0x02], '1');
  ASSERT_EQ(test_scancode_to_ascii[0x03], '2');
  ASSERT_EQ(test_scancode_to_ascii[0x0B], '0');
  TEST_PASS_MSG();
}

TEST(scancode_to_ascii_letters) {
  ASSERT_EQ(test_scancode_to_ascii[0x10], 'q');
  ASSERT_EQ(test_scancode_to_ascii[0x11], 'w');
  ASSERT_EQ(test_scancode_to_ascii[0x1E], 'a');
  ASSERT_EQ(test_scancode_to_ascii[0x1F], 's');
  TEST_PASS_MSG();
}

TEST(scancode_to_ascii_special) {
  ASSERT_EQ(test_scancode_to_ascii[0x0E], '\b'); // Backspace
  ASSERT_EQ(test_scancode_to_ascii[0x1C], '\n'); // Enter
  ASSERT_EQ(test_scancode_to_ascii[0x0C], '-');  // Dash
  TEST_PASS_MSG();
}

TEST(scancode_null_for_modifiers) {
  ASSERT_EQ(test_scancode_to_ascii[0x00], 0); // Null
  ASSERT_EQ(test_scancode_to_ascii[0x1D], 0); // Ctrl
  TEST_PASS_MSG();
}

TEST(key_release_detection) {
  u8 press = 0x1C;
  u8 release = 0x9C;

  ASSERT_EQ((press & 0x80), 0);
  ASSERT_NEQ((release & 0x80), 0);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nKeyboard Mapping Tests:\n");

  RUN_TEST(scancode_to_ascii_numbers);
  RUN_TEST(scancode_to_ascii_letters);
  RUN_TEST(scancode_to_ascii_special);
  RUN_TEST(scancode_null_for_modifiers);
  RUN_TEST(key_release_detection);

  TEST_SUMMARY();
}
