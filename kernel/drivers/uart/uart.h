#pragma once
#include <stdint.h>

struct uart {
  uintptr_t base;
};

void init_uart(struct uart *u, uintptr_t address);
void uart_putchar(struct uart *u, char c);
void uart_putstr(struct uart *u, const char *c);
