#include <console.h>
#include <panic.h>
#include <pmm.h>
#include <string.h>
#include <stdint.h>
#include <vmm.h>
#include <stddef.h>

static uint64_t *root_table;

static inline uint64_t va_vpn(uintptr_t va, int level) {
  return (va >> (12 + 9 * level)) & VPN_MASK;
}

void vmm_init(void) {
  root_table = (uint64_t *)alloc_pmm();
  if (!root_table)
    panic("VMM failed to allocate root table\n");
  memset(root_table, 0, PAGE_SIZE);
  klog(INFO, "VMM initialized with root 0x%lX\n", root_table);
}

int vmm_map_page(uintptr_t va, uintptr_t pa, uint64_t flags) {
  uint64_t *pte = vmm_walk(root_table, va);

  if (pte == NULL)
    return -1;

  // Already allocated
  if (*pte & PTE_V)
    return -1;

  
  
}

uint64_t *vmm_walk(uint64_t *root, uintptr_t va) {
  
}
