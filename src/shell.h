#ifndef SHELL_H
#define SHELL_H

#include "types.h"

int shell_parse_command(char *input, char **cmd, char **args, int *argc);
void shell_init(void);
void shell_run(void);

#endif
