#include <irq/plic.h>
#include <stdint.h>

void plic_init(uintptr_t base, unsigned int context) {
  *(volatile uint32_t *)(base + 0x200000 + context * 0x1000) = 0;
}

void plic_enable(uintptr_t base, unsigned int context, unsigned int irq, unsigned int priority) {
  
}
