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
    pmm.bitmap[pmm.bitmap_size - 1] = UINT64_MAX >> valid_bits;

  klog(INFO,
       "PMM initialized: bitmap [0x%lX, 0x%lX), allocatable [0x%lX, 0x%lX)\n",
       (uintptr_t)pmm.bitmap, pmm.start, pmm.start, pmm.end);
}


/*
We allocate using a bitmap.

A bitmap consists of an array of uint64_t, each bit in the uint64_t represents one page.
Therefore 1 bitmap entry (uint64_t) track 64 page entries, at 4KiB per page its 256KiB tracked per bitmap entry.

As for the bitmap entry itself, it is allocated from the first 0 (free) MSB.
So we allocate from right to left.

Example:

Bitmap entry index = 10 (just arbitrarily chosen)
1111 1111 0111 1001 .....
          ^
          This is the page to be allocated.
*/
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

  for (uint64_t i = 0; i < 64; i++) {
    uint64_t bit = 63 - i;
    
    if (!(bitfield & (1ULL << bit))) {
      pmm.bitmap[idx] |= 1ULL << bit;
      uint64_t page_index = idx * 64 + i;
      return pmm.start + page_index * pmm.page_size;
    }
  }

  return 0;
}

void pmm_free_page(uintptr_t pa) {
  if (pa < pmm.start || pa >= pmm.end) {
    klog(WARN, "pmm tried to free 0x%lX which is out of bounds\n", pa);
    return;
  }

  if ((pa - pmm.start) % pmm.page_size != 0) {
    klog(WARN, "pmm can't free non page size aligned address 0x%lX\n", pa);
    return;
  }

  // Calculate bit position
  uint64_t page_idx = (pa - pmm.start) / pmm.page_size;
  uint64_t bitmap_idx = page_idx / 64;
  uint64_t bit = 63 - (page_idx % 64);

  if (bitmap_idx >= pmm.bitmap_size) {
    klog(WARN, "pmm tried free from bitmap idx %d but it is out of bounds\n", bitmap_idx);
    return;
  }


  // Already free!
  if (!(pmm.bitmap[bitmap_idx] & (1ULL << bit)))
    return;

  // Otherwise clear the page and free
  // At this point the page is 4K aligned
  uint64_t *p = (uint64_t *)pa;
  for (uint64_t i = 0; i < pmm.page_size / sizeof(uint64_t); i++) {
    p[i] = 0;
  }

  // clear bit
  pmm.bitmap[bitmap_idx] &= ~(1ULL << bit);
}
