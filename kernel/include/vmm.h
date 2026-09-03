#pragma once

#include <stdint.h>
#include <stdbool.h>

#define VIRT_UPPER 0xFFFFFFC000000000
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
int vmm_map_range(uintptr_t va, uintptr_t pa, uint64_t size, uint64_t flags);

uint64_t *vmm_walk(uint64_t *root, uintptr_t va, bool alloc);

void vmm_enable(void);

uintptr_t pa_to_va(uintptr_t pa);

