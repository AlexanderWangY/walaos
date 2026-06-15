#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT HUP INT TERM

cat > "$tmpdir/kernel_main_test.c" <<EOF
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int terminal_initialized;
static int keyboard_initialized;
static size_t output_len;
static char output[16];

static const char keyboard_chars[] = { 0, 'h', 0, 'I' };
static size_t keyboard_index;

void terminal_initialize(void) {
  terminal_initialized++;
}

void terminal_putchar(char c) {
  if (output_len >= sizeof(output)) {
    fputs("terminal output overflow\\n", stderr);
    exit(1);
  }

  output[output_len++] = c;
}

void terminal_write(const char *data, size_t size) {
  (void) data;
  (void) size;
  fputs("kernel_main should not write initial terminal text\\n", stderr);
  exit(1);
}

void terminal_writestring(const char *data) {
  (void) data;
  fputs("kernel_main should start with an empty screen\\n", stderr);
  exit(1);
}

void keyboard_initialize(void) {
  keyboard_initialized++;
}

void debug_init(void) {
}

void debug_write(const char *data) {
  (void) data;
}

char keyboard_poll(void) {
  if (keyboard_index < sizeof(keyboard_chars)) {
    return keyboard_chars[keyboard_index++];
  }

  if (terminal_initialized != 1) {
    fprintf(stderr, "expected terminal_initialize once, got %d\\n", terminal_initialized);
    exit(1);
  }

  if (keyboard_initialized != 1) {
    fprintf(stderr, "expected keyboard_initialize once, got %d\\n", keyboard_initialized);
    exit(1);
  }

  if (output_len != 2 || memcmp(output, "hI", 2) != 0) {
    fprintf(stderr, "expected output hI, got %.*s\\n", (int) output_len, output);
    exit(1);
  }

  puts("kernel-main: ok");
  exit(0);
}

void panic(const char *message) {
  fprintf(stderr, "kernel_main panicked: %s\\n", message);
  exit(1);
}

#include "$repo_root/kernel/kernel/kernel.c"

int main(void) {
  kernel_main();

  fputs("kernel_main returned\\n", stderr);
  return 1;
}
EOF

cc -std=gnu11 -Wall -Wextra \
  -I"$repo_root/kernel/include" \
  "$tmpdir/kernel_main_test.c" \
  -o "$tmpdir/kernel_main_test"

"$tmpdir/kernel_main_test"
