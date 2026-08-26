#pragma once

#include <stdint.h>

void plic_init(uintptr_t base, uint32_t context);
void plic_enable(uintptr_t base, uint32_t context, uint32_t irq, uint32_t priority);
uint32_t plic_claim(uintptr_t base, uint32_t context);
void plic_complete(uintptr_t base, uint32_t context, uint32_t irq);
