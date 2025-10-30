#include "../test.h"
#include <string.h>

#define MAX_ARGS 16

static void parse_command_test(char *input, char **cmd, char **args,
                               int *argc) {
  *argc = 0;
  *cmd = input;

  while (*input == ' ')
    input++;
  *cmd = input;

  while (*input && *input != ' ')
    input++;

  if (*input) {
    *input = '\0';
    input++;
  }

  while (*input && *argc < MAX_ARGS) {
    while (*input == ' ')
      input++;
    if (*input == '\0')
      break;

    args[(*argc)++] = input;

    while (*input && *input != ' ')
      input++;
    if (*input) {
      *input = '\0';
      input++;
    }
  }
}

TEST(parse_simple_command) {
  char input[] = "help";
  char *cmd;
  char *args[MAX_ARGS];
  int argc;

  parse_command_test(input, &cmd, args, &argc);

  ASSERT_EQ(strcmp(cmd, "help"), 0);
  ASSERT_EQ(argc, 0);
  TEST_PASS_MSG();
}

TEST(parse_command_with_one_arg) {
  char input[] = "echo hello";
  char *cmd;
  char *args[MAX_ARGS];
  int argc;

  parse_command_test(input, &cmd, args, &argc);

  ASSERT_EQ(strcmp(cmd, "echo"), 0);
  ASSERT_EQ(argc, 1);
  ASSERT_EQ(strcmp(args[0], "hello"), 0);
  TEST_PASS_MSG();
}

TEST(parse_command_with_multiple_args) {
  char input[] = "echo hello world test";
  char *cmd;
  char *args[MAX_ARGS];
  int argc;

  parse_command_test(input, &cmd, args, &argc);

  ASSERT_EQ(strcmp(cmd, "echo"), 0);
  ASSERT_EQ(argc, 3);
  ASSERT_EQ(strcmp(args[0], "hello"), 0);
  ASSERT_EQ(strcmp(args[1], "world"), 0);
  ASSERT_EQ(strcmp(args[2], "test"), 0);
  TEST_PASS_MSG();
}

TEST(parse_command_with_leading_spaces) {
  char input[] = "   help";
  char *cmd;
  char *args[MAX_ARGS];
  int argc;

  parse_command_test(input, &cmd, args, &argc);

  ASSERT_EQ(strcmp(cmd, "help"), 0);
  ASSERT_EQ(argc, 0);
  TEST_PASS_MSG();
}

TEST(parse_command_with_extra_spaces) {
  char input[] = "echo    hello    world";
  char *cmd;
  char *args[MAX_ARGS];
  int argc;

  parse_command_test(input, &cmd, args, &argc);

  ASSERT_EQ(strcmp(cmd, "echo"), 0);
  ASSERT_EQ(argc, 2);
  ASSERT_EQ(strcmp(args[0], "hello"), 0);
  ASSERT_EQ(strcmp(args[1], "world"), 0);
  TEST_PASS_MSG();
}

TEST(parse_empty_command) {
  char input[] = "";
  char *cmd;
  char *args[MAX_ARGS];
  int argc;

  parse_command_test(input, &cmd, args, &argc);

  ASSERT_EQ(cmd[0], '\0');
  ASSERT_EQ(argc, 0);
  TEST_PASS_MSG();
}

int main(void) {
  printf("\nShell Parser Tests:\n");

  RUN_TEST(parse_simple_command);
  RUN_TEST(parse_command_with_one_arg);
  RUN_TEST(parse_command_with_multiple_args);
  RUN_TEST(parse_command_with_leading_spaces);
  RUN_TEST(parse_command_with_extra_spaces);
  RUN_TEST(parse_empty_command);

  TEST_SUMMARY();
}
