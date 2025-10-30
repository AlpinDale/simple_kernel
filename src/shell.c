#include "shell.h"
#include "keyboard.h"
#include "kprint.h"
#include "system.h"
#include "vga.h"

#define CMD_BUFFER_SIZE 256
#define MAX_ARGS 16

static char cmd_buffer[CMD_BUFFER_SIZE];

static void shell_print_prompt(void) {
  kprint_colored("kernel", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
  kprint_colored("> ", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
}

static void parse_command(char *input, char **cmd, char **args, int *argc) {
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

static void cmd_help(void) {
  kprint("Available commands:\n");
  kprint("  help     - Show this help message\n");
  kprint("  clear    - Clear the screen\n");
  kprint("  echo     - Print text\n");
  kprint("  color    - Change text color\n");
  kprint("  rainbow  - Rainbow text demo\n");
  kprint("  version  - Show kernel version\n");
  kprint("  uptime   - Show system uptime\n");
  kprint("  reboot   - Reboot the system\n");
  kprint("  halt     - Halt the system\n");
  kprint("  exit     - Exit the shell\n");
}

static void cmd_clear(void) {
  volatile unsigned short *vga = (volatile unsigned short *)0xB8000;
  for (int i = 0; i < 80 * 25; i++) {
    vga[i] = 0x0F20;
  }
  vga_initialize();
}

static void cmd_echo(char **args, int argc) {
  for (int i = 0; i < argc; i++) {
    kprint(args[i]);
    if (i < argc - 1)
      kprint(" ");
  }
  kprint("\n");
}

static void cmd_color(char **args, int argc) {
  if (argc == 0) {
    kprint("Usage: color <name>\n");
    kprint("Colors: red, green, blue, yellow, cyan, magenta, white\n");
    return;
  }

  u8 color = VGA_COLOR_WHITE;
  char *name = args[0];

  if (name[0] == 'r' && name[1] == 'e')
    color = VGA_COLOR_LIGHT_RED;
  else if (name[0] == 'g')
    color = VGA_COLOR_LIGHT_GREEN;
  else if (name[0] == 'b')
    color = VGA_COLOR_LIGHT_BLUE;
  else if (name[0] == 'y')
    color = VGA_COLOR_YELLOW;
  else if (name[0] == 'c')
    color = VGA_COLOR_LIGHT_CYAN;
  else if (name[0] == 'm')
    color = VGA_COLOR_LIGHT_MAGENTA;
  else if (name[0] == 'w')
    color = VGA_COLOR_WHITE;
  else {
    kprint("Unknown color\n");
    return;
  }

  vga_setcolor(vga_entry_color(color, VGA_COLOR_BLACK));
  kprint("Color changed!\n");
}

static void cmd_rainbow(char **args, int argc) {
  if (argc == 0) {
    kprint_rainbow("Rainbow text demo!\n");
  } else {
    for (int i = 0; i < argc; i++) {
      kprint_rainbow(args[i]);
      if (i < argc - 1)
        kprint_rainbow(" ");
    }
    kprint_rainbow("\n");
  }
}

static void cmd_version(void) {
  kprint("Kernel version: ");
  kprint(KERNEL_VERSION);
  kprint("\n");
  kprint("Built with: GCC + NASM\n");
  kprint("Architecture: x86_64\n");
}

static void cmd_uptime(void) {
  u64 uptime_ms = system_get_uptime_ms();
  u64 seconds = uptime_ms / 1000;
  u64 minutes = seconds / 60;
  u64 hours = minutes / 60;

  seconds %= 60;
  minutes %= 60;

  kprint("Uptime: ");

  if (hours > 0) {
    char h_str[20];
    int i = 0;
    u64 h = hours;
    do {
      h_str[i++] = '0' + (h % 10);
      h /= 10;
    } while (h > 0);
    for (int j = i - 1; j >= 0; j--) {
      char c[2] = {h_str[j], '\0'};
      kprint(c);
    }
    kprint("h ");
  }

  if (minutes > 0 || hours > 0) {
    char m_str[20];
    int i = 0;
    u64 m = minutes;
    do {
      m_str[i++] = '0' + (m % 10);
      m /= 10;
    } while (m > 0);
    for (int j = i - 1; j >= 0; j--) {
      char c[2] = {m_str[j], '\0'};
      kprint(c);
    }
    kprint("m ");
  }

  char s_str[20];
  int i = 0;
  u64 s = seconds;
  do {
    s_str[i++] = '0' + (s % 10);
    s /= 10;
  } while (s > 0);
  for (int j = i - 1; j >= 0; j--) {
    char c[2] = {s_str[j], '\0'};
    kprint(c);
  }
  kprint("s\n");
}

static void cmd_reboot(void) {
  kprint_colored("Rebooting system...\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
  system_reboot();
}

static void cmd_halt(void) {
  kprint_colored("System halted.\n", VGA_COLOR_RED, VGA_COLOR_BLACK);
  system_halt();
}

static int execute_command(char *cmd, char **args, int argc) {
  if (cmd[0] == '\0') {
    return 0;
  }

  if (cmd[0] == 'h' && cmd[1] == 'e')
    cmd_help();
  else if (cmd[0] == 'c' && cmd[1] == 'l')
    cmd_clear();
  else if (cmd[0] == 'e' && cmd[1] == 'c')
    cmd_echo(args, argc);
  else if (cmd[0] == 'c' && cmd[1] == 'o')
    cmd_color(args, argc);
  else if (cmd[0] == 'r' && cmd[1] == 'a')
    cmd_rainbow(args, argc);
  else if (cmd[0] == 'v')
    cmd_version();
  else if (cmd[0] == 'u')
    cmd_uptime();
  else if (cmd[0] == 'r' && cmd[1] == 'e')
    cmd_reboot();
  else if (cmd[0] == 'h' && cmd[1] == 'a')
    cmd_halt();
  else if (cmd[0] == 'e' && cmd[1] == 'x')
    return 1;
  else {
    kprint("Unknown command: ");
    kprint(cmd);
    kprint("\n");
  }

  return 0;
}

void shell_init(void) {
  cmd_clear();
  kprint_rainbow("Welcome to the worst kernel shell!\n");
  kprint("Type 'help' for available commands.\n\n");
}

void shell_run(void) {
  shell_init();

  while (1) {
    shell_print_prompt();

    u16 pos = 0;
    while (1) {
      char c = keyboard_getchar();

      if (c == '\n') {
        kprint("\n");
        cmd_buffer[pos] = '\0';
        break;
      } else if (c == '\b') {
        if (pos > 0) {
          pos--;
          // VGA driver handles backspace
          char bs[2] = {'\b', '\0'};
          vga_writestring(bs);
        }
      } else if (pos < CMD_BUFFER_SIZE - 1) {
        cmd_buffer[pos++] = c;
        char str[2] = {c, '\0'};
        kprint(str);
      }
    }

    char *cmd;
    char *args[MAX_ARGS];
    int argc;

    parse_command(cmd_buffer, &cmd, args, &argc);

    if (execute_command(cmd, args, argc)) {
      kprint_colored("\nExiting shell...\n", VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
      break;
    }
  }
}
