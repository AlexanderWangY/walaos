#include "panic.h"
#include <console.h>
#include <pmm.h>
#include <stdint.h>

static pmm_controller pmm;

void init_pmm(uintptr_t start, uintptr_t end, uint64_t page_size) {
  pmm.start = 0;
  pmm.end = 0;
  pmm.page_size = 0;
  pmm.bitmap = 0;
  pmm.bitmap_size = 0;
  pmm.page_count = 0;

  if (page_size < sizeof(uint64_t) || page_size % sizeof(uint64_t) != 0 ||
      start >= end)
    return;

  // Align the start to be 4K (with page size)
  uintptr_t start_remainder = start % page_size;
  if (start_remainder != 0) {
    uintptr_t adjustment = page_size - start_remainder;
    if (start > UINTPTR_MAX - adjustment) {
      panic("PMM RAM start can not be aligned due to end of address space");
      return;
    }
    start += adjustment;
  }

  // Align end to 4K
  end -= end % page_size;
  if (start >= end) {
    panic("PMM RAM start is past RAM end");
    return;
  }

  // Get # of slots for bitmap
  uint64_t total_page_count = (end - start) / page_size;
  uint64_t bitmap_words = total_page_count / 64;
  if (total_page_count % 64 != 0)
    bitmap_words++;

  // Get number of pages needed for bitmap
  uint64_t bitmap_bytes = bitmap_words * sizeof(uint64_t);
  uint64_t bitmap_pages = bitmap_bytes / page_size;
  if (bitmap_bytes % page_size != 0)
    bitmap_pages++;

  if (bitmap_pages > total_page_count) {
    panic("PMM not enough space in RAM for bitmap");
    return;
  }

  uintptr_t alloc_start = start + bitmap_pages * page_size;

  pmm.start = alloc_start;
  pmm.end = end;
  pmm.page_size = page_size;
  pmm.bitmap = (uint64_t *)start;
  pmm.page_count = (end - alloc_start) / page_size;
  pmm.bitmap_size = pmm.page_count / 64;
  if (pmm.page_count % 64 != 0)
    pmm.bitmap_size++;

  for (uint64_t i = 0; i < pmm.bitmap_size; i++) {
    pmm.bitmap[i] = 0;
  }

  // Mark invalid trailing bits as full so they never get allocated
  uint64_t valid_bits = pmm.page_count % 64;
  if (valid_bits != 0)
    pmm.bitmap[pmm.bitmap_size - 1] = UINT64_MAX << valid_bits;

  klog(INFO,
       "PMM initialized: bitmap [0x%lX, 0x%lX), allocatable [0x%lX, 0x%lX)\n",
       (uintptr_t)pmm.bitmap, pmm.start, pmm.start, pmm.end);
}

uintptr_t alloc_pmm(void) {
  uint64_t idx = pmm.bitmap_size;
  for (uint64_t i = 0; i < pmm.bitmap_size; i++) {
    if (pmm.bitmap[i] != UINT64_MAX) {
      idx = i;
      break;
    }
  }

  if (idx == pmm.bitmap_size)
    return 0;

  uint64_t bitfield = pmm.bitmap[idx];

  for (int i = 63; i >= 0; i--) {
    if (!(bitfield & (1ULL << i))) {
      pmm.bitmap[idx] |= 1ULL << i;
      uint64_t page_index = idx * 64 + (uint64_t)i;
      return pmm.start + page_index * pmm.page_size;
    }
  }

  return 0;
}
