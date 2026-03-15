#include "shell.h"

#include "boot.h"
#include "boot_info.h"
#include "cpuinfo.h"
#include "heap.h"
#include "input.h"
#include "kprint.h"
#include "kstring.h"
#include "panic.h"
#include "pci.h"
#include "pmm.h"
#include "rtc.h"
#include "system.h"
#include "vga.h"

#define CMD_BUFFER_SIZE 256
#define MAX_ARGS 16
#define HISTORY_SIZE 32

typedef void (*shell_command_fn)(char **args, int argc);

typedef struct {
  const char *name;
  shell_command_fn fn;
} shell_command_t;

static char cmd_buffer[CMD_BUFFER_SIZE];
static pci_device_t pci_devices[PCI_MAX_FUNCTIONS];
static char history[HISTORY_SIZE][CMD_BUFFER_SIZE];
static u16 history_head;
static u16 history_count;

static void shell_print_prompt(void) {
  kprint_colored("kernel", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
  kprint_colored("> ", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
}

static size_t history_oldest_index(void) {
  return (history_head + HISTORY_SIZE - history_count) % HISTORY_SIZE;
}

static const char *history_get(size_t position) {
  size_t index = (history_oldest_index() + position) % HISTORY_SIZE;
  return history[index];
}

static void history_add(const char *line) {
  if (line[0] == '\0') {
    return;
  }

  if (history_count > 0) {
    const char *last = history_get(history_count - 1);
    if (kstrcmp(last, line) == 0) {
      return;
    }
  }

  kmemcpy(history[history_head], line, CMD_BUFFER_SIZE);
  history[history_head][CMD_BUFFER_SIZE - 1] = '\0';
  history_head = (history_head + 1) % HISTORY_SIZE;
  if (history_count < HISTORY_SIZE) {
    history_count++;
  }
}

static void shell_set_buffer(const char *text, u16 *pos) {
  while (*pos > 0) {
    kprint("\b");
    (*pos)--;
  }

  for (u16 i = 0; text[i] != '\0' && i + 1 < CMD_BUFFER_SIZE; i++) {
    cmd_buffer[i] = text[i];
    *pos = i + 1;
  }
  cmd_buffer[*pos] = '\0';
  kprint(text);
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
  kprint("  sleep    - Sleep for N milliseconds\n");
  kprint("  time     - Read RTC time\n");
  kprint("  cpuinfo  - Show CPU identification\n");
  kprint("  pci      - Enumerate PCI devices\n");
  kprint("  meminfo  - Show PMM statistics\n");
  kprint("  heapinfo - Show heap statistics\n");
  kprint("  allocdemo- Allocate/free a heap block\n");
  kprint("  mmap     - Show BIOS memory map\n");
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

static void cmd_sleep(char **args, int argc) {
  u64 delay_ms = 100;

  if (argc > 0 && !ku64_parse(args[0], &delay_ms)) {
    kprint("Usage: sleep [ms]\n");
    return;
  }

  kprint("Sleeping for ");
  kprint_u64(delay_ms);
  kprint(" ms...\n");
  system_sleep_ms(delay_ms);
  kprint("Awake\n");
}

static void cmd_time(char **args, int argc) {
  (void)args;
  (void)argc;

  rtc_time_t now;
  if (!rtc_read_time(&now)) {
    kprint("RTC read failed\n");
    return;
  }

  kprint("RTC time: ");
  kprint_u64(now.year);
  kprint("-");
  if (now.month < 10) {
    kprint("0");
  }
  kprint_u64(now.month);
  kprint("-");
  if (now.day < 10) {
    kprint("0");
  }
  kprint_u64(now.day);
  kprint(" ");
  if (now.hour < 10) {
    kprint("0");
  }
  kprint_u64(now.hour);
  kprint(":");
  if (now.minute < 10) {
    kprint("0");
  }
  kprint_u64(now.minute);
  kprint(":");
  if (now.second < 10) {
    kprint("0");
  }
  kprint_u64(now.second);
  kprint("\n");
}

static void cmd_cpuinfo(char **args, int argc) {
  (void)args;
  (void)argc;

  cpu_info_t info;
  cpu_info_read(&info);

  kprint("CPU vendor: ");
  kprint(info.vendor);
  kprint("\nCPU family: ");
  kprint_u64(info.family);
  kprint("\nCPU model: ");
  kprint_u64(info.model);
  kprint("\nCPU stepping: ");
  kprint_u64(info.stepping);
  kprint("\nAPIC present: ");
  kprint(info.apic_present ? "yes" : "no");
  kprint("\nMSR supported: ");
  kprint(info.msr_supported ? "yes" : "no");
  kprint("\n");
}

static void cmd_pci(char **args, int argc) {
  (void)args;
  (void)argc;

  u8 count = pci_enumerate(pci_devices, PCI_MAX_FUNCTIONS);
  kprint("PCI devices: ");
  kprint_u64(count);
  kprint("\n");

  for (u8 i = 0; i < count; i++) {
    kprint("bus=");
    kprint_u64(pci_devices[i].bus);
    kprint(" slot=");
    kprint_u64(pci_devices[i].slot);
    kprint(" func=");
    kprint_u64(pci_devices[i].function);
    kprint(" vendor=");
    kprint_hex(pci_devices[i].vendor_id);
    kprint(" device=");
    kprint_hex(pci_devices[i].device_id);
    kprint(" class=");
    kprint(pci_class_name(pci_devices[i].class_code, pci_devices[i].subclass));
    kprint("\n");
  }
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

static void cmd_heapinfo(char **args, int argc) {
  (void)args;
  (void)argc;
  kprint("Heap total bytes: ");
  kprint_u64(heap_total_bytes());
  kprint("\nHeap used bytes: ");
  kprint_u64(heap_used_bytes());
  kprint("\nHeap free bytes: ");
  kprint_u64(heap_free_bytes());
  kprint("\n");
}

static void cmd_allocdemo(char **args, int argc) {
  u64 size = 64;

  if (argc > 0 && !ku64_parse(args[0], &size)) {
    kprint("Usage: allocdemo [bytes]\n");
    return;
  }

  void *ptr = kmalloc((size_t)size);
  if (ptr == 0) {
    kprint("Allocation failed\n");
    return;
  }

  kmemset(ptr, 0xA5, (size_t)size);
  kprint("Allocated ");
  kprint_u64(size);
  kprint(" bytes at ");
  kprint_hex((u64)(uintptr_t)ptr);
  kprint("\n");

  kfree(ptr);
  kprint("Freed allocation\n");
}

static void cmd_mmap(char **args, int argc) {
  (void)args;
  (void)argc;

  u32 count = 0;
  const memory_map_entry_t *entries = boot_get_memory_map(&count);

  if (entries == 0 || count == 0) {
    kprint("No memory map available\n");
    return;
  }

  for (u32 i = 0; i < count; i++) {
    kprint("[");
    kprint_u64(i);
    kprint("] base=");
    kprint_hex(entries[i].base);
    kprint(" len=");
    kprint_hex(entries[i].length);
    kprint(" type=");
    kprint(boot_memory_type_name(entries[i].type));
    kprint("\n");
  }
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
    {"version", cmd_version}, {"uptime", cmd_uptime}, {"sleep", cmd_sleep},
    {"time", cmd_time},       {"cpuinfo", cmd_cpuinfo}, {"pci", cmd_pci},
    {"meminfo", cmd_meminfo},
    {"heapinfo", cmd_heapinfo}, {"allocdemo", cmd_allocdemo},
    {"mmap", cmd_mmap},       {"panic", cmd_panic},   {"reboot", cmd_reboot},
    {"halt", cmd_halt},
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
    size_t history_position = history_count;
    char current_line[CMD_BUFFER_SIZE];
    current_line[0] = '\0';
    for (;;) {
      input_event_t event = input_getevent();

      if (event.type == INPUT_EVENT_LEFT) {
        vga_scroll_view(-1);
        continue;
      }

      if (event.type == INPUT_EVENT_RIGHT) {
        vga_scroll_view(1);
        continue;
      }

      vga_scroll_to_bottom();

      if (event.type == INPUT_EVENT_UP) {
        if (history_count == 0 || history_position == 0) {
          continue;
        }

        if (history_position == history_count) {
          kmemcpy(current_line, cmd_buffer, CMD_BUFFER_SIZE);
        }

        history_position--;
        shell_set_buffer(history_get(history_position), &pos);
        continue;
      }

      if (event.type == INPUT_EVENT_DOWN) {
        if (history_position >= history_count) {
          continue;
        }

        history_position++;
        if (history_position == history_count) {
          shell_set_buffer(current_line, &pos);
        } else {
          shell_set_buffer(history_get(history_position), &pos);
        }
        continue;
      }

      char c = event.ch;
      if (event.type != INPUT_EVENT_CHAR) {
        continue;
      }

      if (c == '\n') {
        kprint("\n");
        cmd_buffer[pos] = '\0';
        break;
      }

      if (c == '\b' || c == 0x7F) {
        if (pos > 0) {
          if (history_position != history_count) {
            kmemcpy(current_line, cmd_buffer, CMD_BUFFER_SIZE);
            history_position = history_count;
          }
          pos--;
          cmd_buffer[pos] = '\0';
          kprint("\b");
        }
        continue;
      }

      if (pos + 1 < CMD_BUFFER_SIZE) {
        if (history_position != history_count) {
          kmemcpy(current_line, cmd_buffer, CMD_BUFFER_SIZE);
          history_position = history_count;
        }
        cmd_buffer[pos++] = c;
        cmd_buffer[pos] = '\0';
        char tmp[2] = {c, '\0'};
        kprint(tmp);
      }
    }

    history_add(cmd_buffer);

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
