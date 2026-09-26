#include "pmm.h"
#include "vmm.h"
#include <kheap.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <console.h>
#include <panic.h>

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
  
  uintptr_t old_end = kheap_end;
  kblock_t *block_end = kheap;
  while (block_end && block_end->next)
    block_end = block_end->next;
  
  for (size_t i = 0; i < pages; ++i) {
    uintptr_t pa = pmm_alloc_page();
    if (!pa) return false;

    vmm_map_page(KHEAP_BASE + (kheap_pages * PAGE_SIZE), pa, PTE_R | PTE_W);
    kheap_pages += 1;
    kheap_end += PAGE_SIZE;
  }

  klog(INFO, "grew kheap by %d pages\n", pages);

  // If last block is free too, just simply merge and return
  if (block_end->free) {
    block_end->size += pages * PAGE_SIZE;
    return true;
  }

  kblock_t *new_block = (kblock_t *)old_end;
  new_block->free = true;
  new_block->size = (pages * PAGE_SIZE) - HEADER_SIZE;
  new_block->prev = block_end;
  new_block->prev->next = new_block;
  new_block->next = NULL;
  return true;
}

void kheap_init(void) {
  uintptr_t pa = pmm_alloc_page();
  vmm_map_page(KHEAP_BASE, pa, PTE_R | PTE_W);

  kheap = (kblock_t *)KHEAP_BASE;
  
  kheap->free = true;
  kheap->next = NULL;
  kheap->prev = NULL;
  kheap->size = PAGE_SIZE - HEADER_SIZE;

  kheap_end = KHEAP_BASE + PAGE_SIZE;
  kheap_pages = 1;
}

void *kmalloc(size_t size) {
  size_t aligned = align_up_16(size);
  retry: {}

  kblock_t *curr = kheap;

  while (curr && (!curr->free || curr->size < align_up_16(size)))
    curr = curr->next;

  if (curr == NULL) {
    size_t needed = aligned + HEADER_SIZE;
    size_t pages = (needed + PAGE_SIZE - 1) / PAGE_SIZE;
    // This means we no longer have enough space, allocate more!
    bool ok = kheap_grow(pages);
    if (!ok) {
      klog(PANIC, "could not grow heap!");
      panic("");
    }

    goto retry;
  }
  
  size_t remaining = curr->size - aligned;
  if (remaining > HEADER_SIZE + 16) {
    // Allocated and split

    kblock_t *new_block = (kblock_t *)((unsigned char *)curr + HEADER_SIZE + aligned);

    new_block->size = remaining - HEADER_SIZE;
    new_block->free = true;
    new_block->next = curr->next;
    new_block->prev = curr;

    if (new_block->next)
      new_block->next->prev = new_block;

    curr->free = false;
    curr->size = aligned;
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
