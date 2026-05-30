#ifndef _KERNEL_LOG_H
#define _KERNEL_LOG_H

#include <stdio.h>

#define kinfo(fmt, ...)  printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#define kwarn(fmt, ...)  printf("[WARN] " fmt "\n", ##__VA_ARGS__)
#define kerror(fmt, ...) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)

#endif
