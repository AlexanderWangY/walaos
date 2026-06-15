#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT HUP INT TERM

mkdir -p "$tmpdir/include/arch"
cat > "$tmpdir/include/arch/io.h" <<'EOF'
#ifndef TEST_ARCH_IO_H
#define TEST_ARCH_IO_H

#include <stdint.h>

uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t val);
void io_wait(void);

#endif
EOF

cat > "$tmpdir/keyboard_map_test.c" <<EOF
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static const uint8_t *scancode_stream;
static size_t scancode_count;
static size_t scancode_index;

uint8_t inb(uint16_t port) {
  if (port == 0x64) {
    return scancode_index < scancode_count ? 0x01 : 0x00;
  }

  if (port == 0x60) {
    if (scancode_index >= scancode_count) {
      return 0;
    }

    return scancode_stream[scancode_index++];
  }

  return 0;
}

void outb(uint16_t port, uint8_t val) {
  (void) port;
  (void) val;
}

void io_wait(void) {
}

#include "$repo_root/kernel/arch/i386/keyboard.c"

static char poll_scancodes(const uint8_t *codes, size_t count) {
  char out = 0;

  scancode_stream = codes;
  scancode_count = count;
  scancode_index = 0;

  for (size_t i = 0; i < count; i++) {
    out = keyboard_poll();
  }

  return out;
}

static void assert_key(const char *name, const uint8_t *codes, size_t count, char expected) {
  shift = false;

  char actual = poll_scancodes(codes, count);
  if (actual != expected) {
    fprintf(stderr, "%s: expected 0x%02x, got 0x%02x\\n",
        name, (unsigned char) expected, (unsigned char) actual);
    exit(1);
  }
}

#define ASSERT_KEY(name, expected, ...) do { \
  const uint8_t codes[] = { __VA_ARGS__ }; \
  assert_key(name, codes, sizeof(codes) / sizeof(codes[0]), expected); \
} while (0)

int main(void) {
  ASSERT_KEY("normal q", 'q', 0x10);
  ASSERT_KEY("normal m", 'm', 0x32);
  ASSERT_KEY("normal 1", '1', 0x02);
  ASSERT_KEY("normal minus", '-', 0x0C);
  ASSERT_KEY("normal apostrophe", '\\'', 0x28);
  ASSERT_KEY("normal tab", '\\t', 0x0F);
  ASSERT_KEY("normal enter", '\\n', 0x1C);
  ASSERT_KEY("normal space", ' ', 0x39);

  ASSERT_KEY("shift q", 'Q', 0x2A, 0x10);
  ASSERT_KEY("shift m", 'M', 0x2A, 0x32);
  ASSERT_KEY("shift 1", '!', 0x2A, 0x02);
  ASSERT_KEY("shift minus", '_', 0x2A, 0x0C);
  ASSERT_KEY("shift left bracket", '{', 0x2A, 0x1A);
  ASSERT_KEY("shift apostrophe", '"', 0x2A, 0x28);
  ASSERT_KEY("shift slash", '?', 0x2A, 0x35);
  ASSERT_KEY("shift release restores normal", 'q', 0x2A, 0xAA, 0x10);

  puts("keyboard-map: ok");
  return 0;
}
EOF

cc -std=gnu11 -Wall -Wextra \
  -I"$repo_root/kernel/include" \
  -I"$tmpdir/include" \
  "$tmpdir/keyboard_map_test.c" \
  -o "$tmpdir/keyboard_map_test"

"$tmpdir/keyboard_map_test"
