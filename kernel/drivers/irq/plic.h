#pragma once

#include <stdint.h>

void plic_init(uintptr_t base, unsigned context);
void plic_enable(uintptr_t base, unsigned context, unsigned irq, unsigned priority);
unsigned plic_claim(uintptr_t base, unsigned context);
void plic_complete(uintptr_t base, unsigned context, unsigned irq);
