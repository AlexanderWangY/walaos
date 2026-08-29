#pragma once

#include <stdint.h>

const char *platform_name(void);
void platform_init(void);

void platform_console_putc(char c);
int platform_console_getc(void);

uint32_t platform_irq_claim(void);
void platform_irq_complete(uint32_t irq);
