#include <kernel/log.h>
#include <arch/serial.h>

#include <stdarg.h>
#include <stdio.h>

void debug_init(void) {
  serial_init();
}

void debug_write(const char *data) {
  while (*data)
    debug_putchar(*data++);
}

void debug_putchar(char c) {
  if (c == '\n')
    serial_putchar('\r');
  serial_putchar(c);
}

// Adapts void debug_putchar to the int-returning putc_fn sink shape.
static int debug_putc(int c) {
  debug_putchar((char)c);
  return c;
}

int debug_printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int written = vcbprintf(debug_putc, fmt, args);

  va_end(args);
  return written;
}
