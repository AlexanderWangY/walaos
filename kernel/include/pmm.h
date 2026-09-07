#pragma once

#include <stdint.h>

typedef struct {
  uintptr_t start;
  uintptr_t end;
  uint64_t page_size; // Bytes
  uint64_t *bitmap;
  uint64_t bitmap_size;
  uint64_t page_count;
} pmm_controller;

void pmm_init(uintptr_t start, uintptr_t end, uint64_t page_size);
uintptr_t pmm_alloc_page(void);
void pmm_free_page(uintptr_t pa);
void pmm_use_direct_map(void);
