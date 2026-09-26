#pragma once

#include <stddef.h>
#include <stdint.h>

#define KHEAP_BASE      0xffffffd000000000ULL
#define KHEAP_SIZE      (64ULL * 1024 * 1024)

void kheap_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
