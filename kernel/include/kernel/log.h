#ifndef _KERNEL_LOG_H
#define _KERNEL_LOG_H

// NEED THIS FOR kinfo, kwarn, kerror
#include <stdio.h>

#define kinfo(fmt, ...)  printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define kwarn(fmt, ...)  printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#define kerror(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)

// This is for serial debugging
void debug_init(void);
void debug_write(const char* data);
void debug_putchar(char c);

#endif
