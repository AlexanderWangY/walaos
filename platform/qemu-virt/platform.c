#include <platform.h>
#include <stdint.h>
#include <uart/ns16550.h>
#include <irq/plic.h>

#define UART0 0x10000000UL
#define UART0_IRQ 10
#define UART0_PRIORITY 1
#define PLIC_BASE 0x0c000000UL

#define MHART0_CTX 0
#define SHART0_CTX 1
#define MHART1_CTX 2
#define SHART1_CTX 3
// Add more mharts and sharts as we go
// Haha, shart is a funny word... poop

const char *platform_name(void) {
  return "qemu-virt";
}

void platform_init(void) {
    // Enable supervisor hart 0 plic
    plic_init(PLIC_BASE, SHART0_CTX);
    plic_enable(PLIC_BASE, SHART0_CTX, UART0_IRQ, UART0_PRIORITY);
    ns16550_init(UART0);
};

void platform_console_putc(char c) {
    ns16550_putc(UART0, c);
}

int platform_console_getc(void) {
    return ns16550_getc(UART0);
}

uint32_t platform_irq_claim(void) {
    return plic_claim(PLIC_BASE, SHART0_CTX);
}

void platform_irq_complete(uint32_t irq) {
    plic_complete(PLIC_BASE, SHART0_CTX, irq);
}
