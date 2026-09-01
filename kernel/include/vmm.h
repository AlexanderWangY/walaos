#pragma once

#include <stdint.h>

#define PAGE_SIZE 4096

// Page table flag bitmasks
#define PTE_V (1 << 0)
#define PTE_R (1 << 1)
#define PTE_W (1 << 2)
#define PTE_X (1 << 3)

// 0b111111111
#define VPN_MASK 0x1FF

void vmm_init(void);
int vmm_map_page(uintptr_t va, uintptr_t pa, uint64_t flags);
int vmm_unmap_page(uintptr_t va);

uint64_t *vmm_walk(uint64_t *root, uintptr_t va);
