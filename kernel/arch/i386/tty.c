#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>

#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// ----- HELPER FUNCTIONS -------

size_t get_buf_index(size_t x, size_t y) {
  return y * VGA_WIDTH + x;
}

// ------------------------------

void terminal_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  terminal_buffer = VGA_MEMORY;

  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = get_buf_index(x, y);
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }
}

void terminal_setcolor(uint8_t color) {
  terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
  const size_t index = get_buf_index(x, y);
  terminal_buffer[index] = vga_entry(c, color);
}

void terminal_clearrow(size_t y) {
  for (size_t x = 0; x < VGA_WIDTH; x++) {
    size_t idx = get_buf_index(x, y);
    terminal_buffer[idx] = vga_entry(' ', terminal_color);
  }
}

void terminal_scroll() {
  for (size_t y = 1; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      size_t curr_idx = get_buf_index(x, y);
      size_t new_idx = get_buf_index(x, y - 1);
      terminal_buffer[new_idx] = terminal_buffer[curr_idx];
    }
  }

  terminal_clearrow(VGA_HEIGHT - 1);

  terminal_row = VGA_HEIGHT - 1;
  terminal_column = 0;
}


void terminal_putchar(char c) {
  // Handle control characters here
  if (c == '\n') {
    terminal_column = 0;
    terminal_row++;
  } else {
    terminal_putentryat((unsigned char)c, terminal_color, terminal_column, terminal_row);
    terminal_column++;

    if (terminal_column == VGA_WIDTH) {
      terminal_column = 0;
      terminal_row++;
    }
  }

  if (terminal_row == VGA_HEIGHT) {
    terminal_scroll();
  }
}

void terminal_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++)
    terminal_putchar(data[i]);
}

void terminal_writestring(const char *data) {
  terminal_write(data, strlen(data));
}
