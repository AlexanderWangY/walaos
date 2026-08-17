#include <stdint.h>
#include "uart.h"

struct uart init_uart(uintptr_t address) {
  struct uart x = {
    .base = address
  };

  return x;
}


static inline volatile uint8_t *uart_reg(struct uart *u, uintptr_t offset) {
    return (volatile uint8_t *)(u->base + offset);
}

void uart_putchar(struct uart *u, char c) {
  *uart_reg(u, 0) = c;
}

void uart_putstr(struct uart *u, const char *c) {
  while (*c != '\0') {
    uart_putchar(u, *c);
    c++;
  }
}
