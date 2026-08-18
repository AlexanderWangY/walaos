#pragma once

const char *platform_name(void);
void platform_init(void);

void platform_console_putc(char c);
int platform_console_getc(void);
