#ifndef _KERNEL_LOG_H
#define _KERNEL_LOG_H

// NEED THIS FOR kinfo, kwarn, kerror
#include <stdio.h>

#define kinfo(fmt, ...)  printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define kwarn(fmt, ...)  printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#define kerror(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)

// Serial-only: formats like printf but never touches the VGA console.
#define kdebug(fmt, ...) debug_printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)

// This is for serial debugging
void debug_init(void);
void debug_write(const char* data);
void debug_putchar(char c);
int debug_printf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

#endif
