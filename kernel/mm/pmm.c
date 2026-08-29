#include <pmm.h>
#include <stdint.h>

static pmm_controller pmm;

void init_pmm(uintptr_t start, uintptr_t end, uint64_t page_size) {
  pmm.start = start;
  pmm.next = start;
  pmm.end = end;
  pmm.page_size = page_size;
}

uintptr_t alloc_pmm(void) {
  if (pmm.next > pmm.end - pmm.page_size)
    return 0;

  uintptr_t page = pmm.next;
  pmm.next += pmm.page_size;
  return page;
};
