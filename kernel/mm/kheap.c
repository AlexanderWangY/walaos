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

static const size_t HEADER_SIZE = (sizeof(kblock_t) + 15) & ~(size_t)15;

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

  kblock_t *curr = kheap;

  while (curr && (!curr->free || curr->size >= (align_up_16(size) + HEADER_SIZE)))
    curr = curr->next;

  if (curr == NULL)
    return NULL;

  size_t remaining = curr->size - (align_up_16(size) + HEADER_SIZE);
  if (remaining > HEADER_SIZE + 16) {
    // Allocated and split

    kblock_t *new_block = (kblock_t *)(unsigned char *)curr + HEADER_SIZE + curr->size;

    new_block->size = remaining - HEADER_SIZE;
    new_block->free = true;
    new_block->next = curr->next;
    new_block->prev = curr;

    curr->free = false;
    curr->size = align_up_16(size);
    curr->next = new_block;

    return (unsigned char *)curr + HEADER_SIZE;
  } else {
    // handoff entire block, not enough left over to justify splitting
    curr->free = false;
    return (unsigned char *)curr+ HEADER_SIZE;
  }  
}

void kfree(void *ptr) {}
