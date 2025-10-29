#ifndef TEST_H
#define TEST_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_PASS "\033[32mPASS\033[0m"
#define TEST_FAIL "\033[31mFAIL\033[0m"

static int test_count = 0;
static int test_passed = 0;
static int test_failed = 0;

#define TEST(name)                                                             \
  static void test_##name(void);                                               \
  static void run_test_##name(void) {                                          \
    test_count++;                                                              \
    printf("  %-50s", #name);                                                  \
    fflush(stdout);                                                            \
    test_##name();                                                             \
  }                                                                            \
  static void test_##name(void)

#define ASSERT(expr)                                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      printf("%s\n    Assertion failed: %s\n    at %s:%d\n", TEST_FAIL, #expr, \
             __FILE__, __LINE__);                                              \
      test_failed++;                                                           \
      return;                                                                  \
    }                                                                          \
  } while (0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NEQ(a, b) ASSERT((a) != (b))
#define ASSERT_TRUE(expr) ASSERT((expr) == true)
#define ASSERT_FALSE(expr) ASSERT((expr) == false)
#define ASSERT_NULL(ptr) ASSERT((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT((ptr) != NULL)

#define TEST_PASS_MSG()                                                        \
  do {                                                                         \
    printf("%s\n", TEST_PASS);                                                 \
    test_passed++;                                                             \
  } while (0)

#define RUN_TEST(name) run_test_##name()

#define TEST_SUMMARY()                                                         \
  do {                                                                         \
    printf("\n");                                                              \
    printf("Tests run: %d\n", test_count);                                     \
    printf("Passed: \033[32m%d\033[0m\n", test_passed);                        \
    if (test_failed > 0) {                                                     \
      printf("Failed: \033[31m%d\033[0m\n", test_failed);                      \
    }                                                                          \
    return (test_failed == 0) ? 0 : 1;                                         \
  } while (0)

#endif
