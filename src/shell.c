#include "shell.h"

#include "input.h"
#include "kprint.h"
#include "kstring.h"
#include "panic.h"
#include "pmm.h"
#include "system.h"
#include "vga.h"

#define CMD_BUFFER_SIZE 256
#define MAX_ARGS 16

typedef void (*shell_command_fn)(char **args, int argc);

typedef struct {
  const char *name;
  shell_command_fn fn;
} shell_command_t;

static char cmd_buffer[CMD_BUFFER_SIZE];

static void shell_print_prompt(void) {
  kprint_colored("kernel", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
  kprint_colored("> ", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
}

int shell_parse_command(char *input, char **cmd, char **args, int *argc) {
  *argc = 0;

  while (*input == ' ') {
    input++;
  }

  *cmd = input;

  while (*input != '\0' && *input != ' ') {
    input++;
  }

  if (*input != '\0') {
    *input++ = '\0';
  }

  while (*input != '\0' && *argc < MAX_ARGS) {
    while (*input == ' ') {
      input++;
    }

    if (*input == '\0') {
      break;
    }

    args[(*argc)++] = input;

    while (*input != '\0' && *input != ' ') {
      input++;
    }

    if (*input != '\0') {
      *input++ = '\0';
    }
  }

  return *argc;
}

static void cmd_help(char **args, int argc) {
  (void)args;
  (void)argc;
  kprint("Available commands:\n");
  kprint("  help     - Show this help message\n");
  kprint("  clear    - Clear the screen\n");
  kprint("  echo     - Print text\n");
  kprint("  color    - Change text color\n");
  kprint("  rainbow  - Rainbow text demo\n");
  kprint("  version  - Show kernel version\n");
  kprint("  uptime   - Show system uptime\n");
  kprint("  meminfo  - Show PMM statistics\n");
  kprint("  panic    - Trigger a panic for testing\n");
  kprint("  reboot   - Reboot the system\n");
  kprint("  halt     - Halt the system\n");
  kprint("  exit     - Exit QEMU successfully\n");
}

static void cmd_clear(char **args, int argc) {
  (void)args;
  (void)argc;
  vga_clear();
}

static void cmd_echo(char **args, int argc) {
  for (int i = 0; i < argc; i++) {
    kprint(args[i]);
    if (i + 1 < argc) {
      kprint(" ");
    }
  }
  kprint("\n");
}

static void cmd_color(char **args, int argc) {
  if (argc == 0) {
    kprint("Usage: color <name>\n");
    return;
  }

  u8 color = VGA_COLOR_WHITE;

  if (kstrcmp(args[0], "red") == 0) {
    color = VGA_COLOR_LIGHT_RED;
  } else if (kstrcmp(args[0], "green") == 0) {
    color = VGA_COLOR_LIGHT_GREEN;
  } else if (kstrcmp(args[0], "blue") == 0) {
    color = VGA_COLOR_LIGHT_BLUE;
  } else if (kstrcmp(args[0], "yellow") == 0) {
    color = VGA_COLOR_YELLOW;
  } else if (kstrcmp(args[0], "cyan") == 0) {
    color = VGA_COLOR_LIGHT_CYAN;
  } else if (kstrcmp(args[0], "magenta") == 0) {
    color = VGA_COLOR_LIGHT_MAGENTA;
  } else if (kstrcmp(args[0], "white") == 0) {
    color = VGA_COLOR_WHITE;
  } else {
    kprint("Unknown color\n");
    return;
  }

  vga_setcolor(vga_entry_color(color, VGA_COLOR_BLACK));
  kprint("Color changed!\n");
}

static void cmd_rainbow(char **args, int argc) {
  if (argc == 0) {
    kprint_rainbow("Rainbow text demo!\n");
    return;
  }

  for (int i = 0; i < argc; i++) {
    kprint_rainbow(args[i]);
    if (i + 1 < argc) {
      kprint_rainbow(" ");
    }
  }
  kprint_rainbow("\n");
}

static void cmd_version(char **args, int argc) {
  (void)args;
  (void)argc;
  kprint("Kernel version: ");
  kprint(KERNEL_VERSION);
  kprint("\nArchitecture: x86_64\n");
}

static void cmd_uptime(char **args, int argc) {
  (void)args;
  (void)argc;
  kprint("Uptime (ms): ");
  kprint_u64(system_get_uptime_ms());
  kprint("\n");
}

static void cmd_meminfo(char **args, int argc) {
  (void)args;
  (void)argc;
  kprint("PMM total bytes: ");
  kprint_u64(pmm_total_bytes());
  kprint("\nPMM free bytes: ");
  kprint_u64(pmm_free_bytes());
  kprint("\nPMM reserved bytes: ");
  kprint_u64(pmm_reserved_bytes());
  kprint("\n");
}

static void cmd_panic(char **args, int argc) {
  (void)args;
  (void)argc;
  panic("panic command invoked");
}

static void cmd_reboot(char **args, int argc) {
  (void)args;
  (void)argc;
  kprint_colored("Rebooting system...\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
  system_reboot();
}

static void cmd_halt(char **args, int argc) {
  (void)args;
  (void)argc;
  kprint_colored("System halted.\n", VGA_COLOR_RED, VGA_COLOR_BLACK);
  system_halt();
}

static const shell_command_t commands[] = {
    {"help", cmd_help},       {"clear", cmd_clear},   {"echo", cmd_echo},
    {"color", cmd_color},     {"rainbow", cmd_rainbow},
    {"version", cmd_version}, {"uptime", cmd_uptime}, {"meminfo", cmd_meminfo},
    {"panic", cmd_panic},     {"reboot", cmd_reboot}, {"halt", cmd_halt},
};

static int execute_command(char *cmd, char **args, int argc) {
  if (cmd[0] == '\0') {
    return 0;
  }

  if (kstrcmp(cmd, "exit") == 0) {
    return 1;
  }

  for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
    if (kstrcmp(cmd, commands[i].name) == 0) {
      commands[i].fn(args, argc);
      return 0;
    }
  }

  kprint("Unknown command: ");
  kprint(cmd);
  kprint("\n");
  return 0;
}

void shell_init(void) {
  cmd_clear(0, 0);
  kprint_rainbow("Welcome to the kernel shell!\n");
  kprint("Type 'help' for available commands.\n\n");
}

void shell_run(void) {
  shell_init();

  for (;;) {
    shell_print_prompt();

    u16 pos = 0;
    for (;;) {
      char c = input_getchar();

      if (c == '\n') {
        kprint("\n");
        cmd_buffer[pos] = '\0';
        break;
      }

      if (c == '\b' || c == 0x7F) {
        if (pos > 0) {
          pos--;
          kprint("\b");
        }
        continue;
      }

      if (pos + 1 < CMD_BUFFER_SIZE) {
        cmd_buffer[pos++] = c;
        char tmp[2] = {c, '\0'};
        kprint(tmp);
      }
    }

    char *cmd = 0;
    char *args[MAX_ARGS];
    int argc = 0;
    shell_parse_command(cmd_buffer, &cmd, args, &argc);

    if (execute_command(cmd, args, argc)) {
      kprint_colored("Exiting shell.\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
      system_qemu_exit(0x10);
      return;
    }
  }
}
