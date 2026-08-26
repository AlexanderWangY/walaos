#include <irq/plic.h>
#include <stdint.h>

// Sets the context's priority threshold to 0
void plic_init(uintptr_t base, uint32_t context) {
  *(volatile uint32_t *)(base + 0x200000 + context * 0x1000) = 0;
}

// Set the enable bit of the irq to 1 and priority
void plic_enable(uintptr_t base, uint32_t context, uint32_t irq, uint32_t priority) {
  volatile uint32_t *enable = (volatile uint32_t *)(base + 0x002000 + context * 0x80 + (irq / 32) * 4);
  *enable |= (1U << (irq % 32));
  *(volatile uint32_t *)(base + irq * 4) = priority;
}

uint32_t plic_claim(uintptr_t base, uint32_t context) {
    return *(volatile uint32_t *)(base + 0x200004 + context * 0x1000);
}

void plic_complete(uintptr_t base, uint32_t context, uint32_t irq) {
  *(volatile uint32_t *)(base + 0x200004 + context * 0x1000) = irq;
}

