#include "pmm.h"
#include "vmm.h"
#include <kheap.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

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

static kblock_t *kheap;

void kheap_init(void) {
  uintptr_t pa = pmm_alloc_page();

  kheap = (kblock_t*)pa_to_va(pa);
  kheap->free = true;
  kheap->next = NULL;
  kheap->prev = NULL;
  kheap->size = PAGE_SIZE - align_up_16(sizeof(kblock_t));
}

void *kmalloc(size_t size) {
  // Need to fit size + kblock_t
  size_t alloc_size = align_up_16(sizeof(kblock_t) + size);

  kblock_t *curr = kheap;
  while (curr && !curr->free && curr->size >= alloc_size) curr = curr->next;

  // Failed to malloc a page
  if (curr == NULL) return 0;

  size_t remaining = curr->size - alloc_size;

  if (remaining >= align_up_16(sizeof(kblock_t)) + 16) {
    kblock_t *new_block = (kblock_t *)(unsigned char *)curr + alloc_size;

    new_block->size = remaining - align_up_16(sizeof(kblock_t));
    new_block->free = true;
    new_block->next = curr->next;
    new_block->prev = curr;

    curr->free = false;
    curr->next = new_block;
    curr->size = alloc_size;
  } else {
    // just give the entire block
    curr->free = false;
    return (unsigned char *)curr + align_up_16(sizeof(*curr));
  }
  
}

void kfree(void *ptr) {}
