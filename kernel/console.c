#include "include/console.h"
#include "include/platform.h"

void console_putc(char c) {
    if (c == '\n')
        platform_console_putc('\r');
    platform_console_putc(c);
}

void console_puts(const char *s) {
    while (*s)
        console_putc(*s++);
}
