#include <uart/ns16550.h>
#include <stdint.h>

#define RBR 0 // Receive buffer
#define THR 0 // Transmit holding
#define IER 1 // Interrupt enable
#define FCR 2 // FIFO control
#define LCR 3 // Line control
#define LSR 5 // Line status

#define LSR_DR (1 << 0) // Data ready
#define LSR_THRE (1 << 5) // Transmit holding empty

static inline volatile uint8_t *reg(uintptr_t base, int offset) {
    return (volatile uint8_t *)(base + offset);
}

void ns16550_init(uintptr_t base) {
    *reg(base, IER) = 0x01; // Enabled interrupt
    *reg(base, LCR) = 0x03;
    *reg(base, FCR) = 0x01; // Enable FIFO

    // Add baud divisor and clock hz stuff later, im too lazy
}

void ns16550_putc(uintptr_t base, char c) {
    while ((*reg(base, LSR) & LSR_THRE) == 0);
    *reg(base, THR) = c;
}

int ns16550_getc(uintptr_t base) {
    while((*reg(base, LSR) & LSR_DR) == 0);
    return *reg(base, RBR);
}
