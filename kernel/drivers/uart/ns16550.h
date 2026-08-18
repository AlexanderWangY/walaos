#pragma once

#include <stdint.h>

void ns16550_init(uintptr_t base);
void ns16550_putc(uintptr_t base, char c);
int ns16550_getc(uintptr_t base);
