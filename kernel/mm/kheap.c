#include "pmm.h"
#include "vmm.h"
#include <kheap.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <console.h>

static inline size_t align_up_16(size_t value) {
  return (value + 15) & ~(size_t)15;
}

// what the ugly C
typedef struct kblock_t {
  size_t size;
  bool free;
  struct kblock_t *next;
  struct kblock_t *prev;
} kblock_t;

static const size_t HEADER_SIZE = (sizeof(kblock_t) + 15) & ~(size_t)15;

static kblock_t *kheap;
static size_t kheap_pages;
static uintptr_t kheap_end;

static bool kheap_grow(size_t pages) {
  // First check if within bounds
  if (kheap_end + pages * PAGE_SIZE > KHEAP_BASE + KHEAP_SIZE) {
      klog(WARN, "Can't grow kheap. kheap is capped at %d MiB",
           KHEAP_SIZE / (1024 * 1024));
      return false;
  }
  
  kblock_t *block_end = kheap;
  while (block_end && block_end->next)
    block_end = block_end->next;
  
  for (size_t i = 0; i < pages; ++i) {
    uintptr_t pa = pmm_alloc_page();
    if (!pa) return false;

    vmm_map_page(KHEAP_BASE + (kheap_pages * PAGE_SIZE), pa, PTE_R | PTE_W | PTE_X);
    kheap_pages += 1;
    kheap_end += PAGE_SIZE;
  }
}

void kheap_init(void) {
  uintptr_t pa = pmm_alloc_page();
  vmm_map_page(KHEAP_BASE, pa, PTE_R | PTE_W | PTE_X);

  kheap = (kblock_t *)KHEAP_BASE;
  
  kheap->free = true;
  kheap->next = NULL;
  kheap->prev = NULL;
  kheap->size = PAGE_SIZE - HEADER_SIZE;

  kheap_end = KHEAP_BASE + PAGE_SIZE;
  kheap_pages = 1;
}

void *kmalloc(size_t size) {
  kblock_t *curr = kheap;

  while (curr && (!curr->free || curr->size < align_up_16(size)))
    curr = curr->next;

  if (curr == NULL) {
    // This means we no longer have enough space, allocate more!
    kheap_grow(size / PAGE_SIZE);
    // TODO: Continue here
  }
  
  size_t remaining = curr->size - align_up_16(size);
  if (remaining > HEADER_SIZE + 16) {
    // Allocated and split

    kblock_t *new_block = (kblock_t *)((unsigned char *)curr + HEADER_SIZE + align_up_16(size));

    new_block->size = remaining - HEADER_SIZE;
    new_block->free = true;
    new_block->next = curr->next;
    new_block->prev = curr;

    if (new_block->next)
      new_block->next->prev = new_block;

    curr->free = false;
    curr->size = align_up_16(size);
    curr->next = new_block;

    return (unsigned char *)curr + HEADER_SIZE;
  } else {
    // handoff entire block, not enough left over to justify splitting
    curr->free = false;
    return (unsigned char *)curr + HEADER_SIZE;
  }  
}

void kfree(void *ptr) {
  if (ptr == NULL) return;

  // 1. cast to kblock_t
  kblock_t *block = (kblock_t *)((unsigned char *)ptr - HEADER_SIZE);
  // 2. free the block
  block->free = true;
  // 3. merge sections (prev and next)

  // Merge previous
  if (block->prev && block->prev->free) {
    // New block has size this size + HEADER SIZE
    block->prev->size += HEADER_SIZE + block->size;
    block->prev->next = block->next;
    block = block->prev;
  }

  // Merge next
  if (block->next && block->next->free) {
    block->size += HEADER_SIZE + block->next->size;
    block->next = block->next->next;
  }
}
