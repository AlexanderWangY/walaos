#pragma once

#include <stdarg.h>

enum LOG_LEVEL {
  DEBUG = 0,
  INFO,
  WARN,
  PANIC,  
};

void console_putc(char c);
void console_puts(const char *s);
void vkprintf(const char *s, va_list args);
void kprintf(const char *s, ...);
void set_log_lvl(enum LOG_LEVEL lvl);
void klog(enum LOG_LEVEL lvl, const char *s, ...);
