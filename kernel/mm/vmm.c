#include <console.h>
#include <panic.h>
#include <pmm.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <vmm.h>
#include <stddef.h>

static uint64_t *root_table;
static bool paging_enabled = false;

static inline uint64_t *table_ptr(uintptr_t pa) {
  if (paging_enabled)
    return (uint64_t *)pa_to_va(pa);

  return (uint64_t *)pa;
}

static inline uint64_t va_vpn(uintptr_t va, int level) {
  return (va >> (12 + 9 * level)) & VPN_MASK;
}

static uint64_t *vmm_walk(uint64_t *root, uintptr_t va, bool alloc);

void vmm_init(void) {
  root_table = (uint64_t *)pmm_alloc_page();
  if (!root_table)
    panic("VMM failed to allocate root table\n");
  memset(root_table, 0, PAGE_SIZE);
  klog(INFO, "VMM initialized with root 0x%lX\n", root_table);
}

int vmm_map_page(uintptr_t va, uintptr_t pa, uint64_t flags) {  
  if ((va & (PAGE_SIZE - 1)) != 0)
    return -1;

  if ((pa & (PAGE_SIZE - 1)) != 0)
    return -1;
  
  uint64_t *pte = vmm_walk(root_table, va, true);

  if (pte == NULL)
    return -1;

  // Already allocated
  if (*pte & PTE_V)
    return -1;

  *pte = ((pa >> 12) << 10) | flags | PTE_V;
  return 0;
}

int vmm_map_range(uintptr_t va, uintptr_t pa, uint64_t size, uint64_t flags) {
  for (uint64_t i = 0; i < size; i += PAGE_SIZE) {
    if (vmm_map_page(va + i, pa + i, flags) != 0)
      return -1;
  }
  
  return 0;
}

static uint64_t *vmm_walk(uint64_t *root, uintptr_t va, bool alloc) {

  uint64_t vpn2 = va_vpn(va, 2);
  uint64_t vpn1 = va_vpn(va, 1);
  uint64_t vpn0 = va_vpn(va, 0);

  uint64_t *pte2 = &root[vpn2];

  if (!(*pte2 & PTE_V)) {
    if (!alloc)
      return NULL;

    uintptr_t pa = pmm_alloc_page();
    if (!pa)
      return NULL;
    memset(table_ptr(pa), 0, PAGE_SIZE);
    *pte2 = ((pa >> 12) << 10) | PTE_V;
  }

  // Superpage leaf, not a table pointer. Not supported yet.
  // TODO: handle Sv39 gigapage (level-2) / megapage (level-1) leaves instead of bailing.
  if (*pte2 & (PTE_W | PTE_R | PTE_X))
    return NULL;

  uintptr_t pa1 = (*pte2 >> 10) << 12;
  uint64_t *table1 = table_ptr(pa1);
  uint64_t *pte1 = &table1[vpn1];

  if (!(*pte1 & PTE_V)) {
    if (!alloc)
      return NULL;

    uintptr_t pa = pmm_alloc_page();
    if (!pa)
      return NULL;
    memset(table_ptr(pa), 0, PAGE_SIZE);
    *pte1 = ((pa >> 12) << 10) | PTE_V;
  }

  // Superpage leaf, not a table pointer. Not supported yet.
  if (*pte1 & (PTE_W | PTE_R | PTE_X))
    return NULL;

  
  uintptr_t pa0 = (*pte1 >> 10) << 12;
  uint64_t *table0 = table_ptr(pa0);
  return &table0[vpn0];
}

int vmm_unmap_page(uintptr_t va) {
  if (va % PAGE_SIZE != 0) {
    klog(WARN, "vmm tried to unmap poorly aligned virtual address 0x%lX\n", va);
    return -1;
  }

  uint64_t *pte = vmm_walk(root_table, va, false);
  if (pte == NULL || !(*pte & PTE_V)) {
    klog(WARN, "vmm cannot unmap unmapped virtual address 0x%lX\n", va);
    return -1;
  }

  *pte = 0;

  // Flush the affected virtual address
  __asm__ volatile(
    "sfence.vma %0, zero"
    :
    : "r"(va)
    : "memory"
  );
  
  return 0;
}

void vmm_enable(void) {
  uintptr_t root_pa = (uintptr_t)root_table;
  uint64_t satp = (8ULL << 60) | (root_pa >> 12);

  __asm__ volatile(
    "csrw satp, %0\n"
    "sfence.vma"
    :
    : "r"(satp)
    : "memory"
  );

  root_table = (uint64_t *)pa_to_va(root_pa);
  paging_enabled = true;
}

uintptr_t pa_to_va(uintptr_t pa) {
  return VIRT_UPPER + pa;
}
