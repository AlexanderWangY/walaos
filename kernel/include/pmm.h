#pragma once

#include <stdint.h>

typedef struct {
  uintptr_t start;
  uintptr_t next;
  uintptr_t end;
  uint64_t page_size; // Bytes
} pmm_controller;

void init_pmm(uintptr_t start, uintptr_t end, uint64_t page_size);
uintptr_t alloc_pmm(void);

