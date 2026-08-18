#include "../../kernel/include/platform.h"
#include "../../kernel/drivers/uart/ns16550.h"

#define UART0 0x10000000UL

const char *platform_name(void) {
  return "qemu-virt";
}

void platform_init(void) {
    ns16550_init(UART0);
};

void platform_console_putc(char c) {
    ns16550_putc(UART0, c);
}

int platform_console_getc(void) {
    return ns16550_getc(UART0);
}
