#include "../test.h"

#include "../../src/kstring.h"

TEST(kstrcmp_matches_exactly) {
  ASSERT_EQ(kstrcmp("help", "help"), 0);
  ASSERT_TRUE(kstrcmp("help", "hel") > 0);
  TEST_PASS_MSG();
}

TEST(kmemmove_handles_overlap) {
  char data[8] = "abcdef";
  kmemmove(data + 2, data, 4);
  ASSERT_EQ(kstrcmp(data, "ababcd"), 0);
  TEST_PASS_MSG();
}

TEST(ku64_to_hex_formats_values) {
  char buffer[32];
  ASSERT_EQ(kstrcmp(ku64_to_hex(buffer, sizeof(buffer), 0x2A), "2A"), 0);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nKString Tests:\n");
  RUN_TEST(kstrcmp_matches_exactly);
  RUN_TEST(kmemmove_handles_overlap);
  RUN_TEST(ku64_to_hex_formats_values);
  TEST_SUMMARY();
}
