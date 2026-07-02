#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>
#include <stdarg.h>

#define EOF (-1)

// Character sink for vcbprintf: consumes one char, returns it or EOF on failure.
typedef int (*putc_fn)(int);

int printf(const char* __restrict, ...);
int vcbprintf(putc_fn put, const char* __restrict format, va_list parameters);
int putchar(int);
int puts(const char*);

#endif
