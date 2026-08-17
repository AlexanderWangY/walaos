#pragma once
#include "../drivers/uart/uart.h"

void console_init(void);
void console_putc(char c);
void console_puts(const char *s);
