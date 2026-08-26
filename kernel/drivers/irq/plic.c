#include <irq/plic.h>
#include <stdint.h>

// Sets the context's priority threshold to 0
void plic_init(uintptr_t base, unsigned int context) {
  *(volatile uint32_t *)(base + 0x200000 + context * 0x1000) = 0;
}

// Set the enable bit of the irq to 1 and priority
void plic_enable(uintptr_t base, unsigned int context, unsigned int irq, unsigned int priority) {
  volatile uint32_t *enable = (volatile uint32_t *)(base + 0x002000 + context * 0x80 + (irq / 32) * 4);
  *enable |= (1U << (irq % 32));
  *(volatile uint32_t *)(base + irq * 4) = priority;
}


