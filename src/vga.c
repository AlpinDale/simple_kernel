#include "vga.h"
#include "io.h"
#include "kstring.h"

#define VGA_SCROLLBACK_ROWS 256

static size_t terminal_row;
static size_t terminal_column;
static size_t viewport_row;
static size_t total_rows;
static u8 terminal_color;
static u16 *terminal_buffer;
static u16 history_buffer[VGA_SCROLLBACK_ROWS * VGA_CONTENT_WIDTH];

static size_t vga_bottom_viewport_row(void) {
  if (total_rows > VGA_CONTENT_HEIGHT) {
    return total_rows - VGA_CONTENT_HEIGHT;
  }
  return 0;
}

static u16 blank_entry(void) { return vga_entry(' ', terminal_color); }

static void clear_history_row(size_t row) {
  u16 blank = blank_entry();
  for (size_t i = 0; i < VGA_CONTENT_WIDTH; i++) {
    history_buffer[row * VGA_CONTENT_WIDTH + i] = blank;
  }
}

static void shift_history_up(void) {
  kmemmove(history_buffer, history_buffer + VGA_CONTENT_WIDTH,
           (VGA_SCROLLBACK_ROWS - 1) * VGA_CONTENT_WIDTH * sizeof(u16));
  clear_history_row(VGA_SCROLLBACK_ROWS - 1);

  if (terminal_row > 0) {
    terminal_row--;
  }
  if (viewport_row > 0) {
    viewport_row--;
  }
  if (total_rows > 0) {
    total_rows--;
  }
}

static void render_viewport(void) {
  u16 blank = blank_entry();

  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    size_t dst_index = y * VGA_WIDTH;
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      terminal_buffer[dst_index + x] = blank;
    }
  }

  for (size_t y = 0; y < VGA_CONTENT_HEIGHT; y++) {
    size_t src_row = viewport_row + y;
    size_t dst_row = y + VGA_PADDING_Y;

    if (src_row >= total_rows) {
      continue;
    }

    kmemcpy(terminal_buffer + dst_row * VGA_WIDTH + VGA_PADDING_X,
            history_buffer + src_row * VGA_CONTENT_WIDTH,
            VGA_CONTENT_WIDTH * sizeof(u16));
  }
}

static void update_cursor(void) {
  size_t visible_row = VGA_CONTENT_HEIGHT - 1;

  if (terminal_row >= viewport_row &&
      terminal_row < viewport_row + VGA_CONTENT_HEIGHT) {
    visible_row = terminal_row - viewport_row;
  }

  u16 pos = (u16)((visible_row + VGA_PADDING_Y) * VGA_WIDTH +
                  terminal_column + VGA_PADDING_X);
  outb(0x3D4, 0x0F);
  outb(0x3D5, (u8)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

static void sync_viewport(int follow_bottom) {
  if (follow_bottom) {
    viewport_row = vga_bottom_viewport_row();
  } else if (viewport_row > vga_bottom_viewport_row()) {
    viewport_row = vga_bottom_viewport_row();
  }

  render_viewport();
  update_cursor();
}

static void advance_line(int follow_bottom) {
  terminal_column = 0;
  terminal_row++;

  if (terminal_row >= VGA_SCROLLBACK_ROWS) {
    shift_history_up();
  }

  if (terminal_row + 1 > total_rows) {
    total_rows = terminal_row + 1;
  }

  clear_history_row(terminal_row);
  sync_viewport(follow_bottom);
}

void vga_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  viewport_row = 0;
  total_rows = 1;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  if (terminal_buffer == 0) {
    terminal_buffer = (u16 *)VGA_MEMORY;
  }
  vga_clear();
  update_cursor();
}

void vga_setcolor(u8 color) { terminal_color = color; }

void vga_putentryat(char c, u8 color, size_t x, size_t y) {
  if (viewport_row + y >= VGA_SCROLLBACK_ROWS || x >= VGA_CONTENT_WIDTH ||
      y >= VGA_CONTENT_HEIGHT) {
    return;
  }

  history_buffer[(viewport_row + y) * VGA_CONTENT_WIDTH + x] =
      vga_entry(c, color);
  render_viewport();
}

void vga_putchar(char c) {
  int follow_bottom = viewport_row == vga_bottom_viewport_row();

  if (c == '\n') {
    advance_line(follow_bottom);
    return;
  }

  if (c == '\b') {
    if (terminal_column > 0 || terminal_row > 0) {
      if (terminal_column == 0) {
        terminal_row--;
        terminal_column = VGA_CONTENT_WIDTH;
      }
      terminal_column--;
      history_buffer[terminal_row * VGA_CONTENT_WIDTH + terminal_column] =
          vga_entry(' ', terminal_color);
    }
    sync_viewport(follow_bottom);
    return;
  }

  history_buffer[terminal_row * VGA_CONTENT_WIDTH + terminal_column] =
      vga_entry(c, terminal_color);

  if (++terminal_column == VGA_CONTENT_WIDTH) {
    advance_line(follow_bottom);
    return;
  }

  if (terminal_row + 1 > total_rows) {
    total_rows = terminal_row + 1;
  }
  sync_viewport(follow_bottom);
}

void vga_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++) {
    vga_putchar(data[i]);
  }
}

void vga_writestring(const char *data) {
  vga_write(data, kstrlen(data));
}

void vga_clear(void) {
  for (size_t row = 0; row < VGA_SCROLLBACK_ROWS; row++) {
    clear_history_row(row);
  }

  terminal_row = 0;
  terminal_column = 0;
  viewport_row = 0;
  total_rows = 1;
  render_viewport();
  update_cursor();
}

void vga_bind_buffer(u16 *buffer) { terminal_buffer = buffer; }

void vga_scroll_view(i32 delta) {
  i32 next = (i32)viewport_row + delta;
  i32 bottom = (i32)vga_bottom_viewport_row();

  if (next < 0) {
    next = 0;
  }
  if (next > bottom) {
    next = bottom;
  }

  viewport_row = (size_t)next;
  render_viewport();
  update_cursor();
}

void vga_scroll_to_bottom(void) {
  viewport_row = vga_bottom_viewport_row();
  render_viewport();
  update_cursor();
}

size_t vga_get_row(void) {
  if (terminal_row >= viewport_row &&
      terminal_row < viewport_row + VGA_CONTENT_HEIGHT) {
    return terminal_row - viewport_row;
  }
  return VGA_CONTENT_HEIGHT - 1;
}

size_t vga_get_column(void) { return terminal_column; }

size_t vga_get_viewport_row(void) { return viewport_row; }

size_t vga_get_total_rows(void) { return total_rows; }

u8 vga_get_color(void) { return terminal_color; }
