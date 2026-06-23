#include <arch/gdt.h>
#include <kernel/log.h>
#include <stdint.h>

#define GDT_ENTRIES 5

struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));

struct gdtr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

static void gdt_set_gate(
  struct gdt_entry *gdt,
  int index,
  uint32_t base,
  uint32_t limit,
  uint8_t access,
  uint8_t flags
) {

  gdt[index].base_low = base & 0xFFFF;
  gdt[index].base_middle = (base >> 16) & 0xFF;
  gdt[index].base_high = (base >> 24) & 0xFF;

  gdt[index].limit_low = limit & 0xFFFF;
  gdt[index].granularity = (limit >> 16) & 0x0F;
  gdt[index].granularity |= flags & 0xF0;

  
  gdt[index].access = access;
}

extern void gdt_flush(uint32_t gdtr_address);

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdtr gdt_ptr;

void init_gdt(void) {
  // Null descriptor
  gdt_set_gate(gdt, 0, 0, 0, 0x00, 0x00);

  // Kernel code
  gdt_set_gate(gdt, 1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

  // Kernel data
  gdt_set_gate(gdt, 2, 0, 0xFFFFFFFF, 0x92, 0xCF);

  // User code
  gdt_set_gate(gdt, 3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

  // User data
  gdt_set_gate(gdt, 4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

  // Load into gdt_ptr
  gdt_ptr.limit = sizeof(gdt) - 1;
  gdt_ptr.base = (uint32_t)&gdt;

  // Use externed assembly to reset registers and enter protected mode
  gdt_flush((uint32_t)&gdt_ptr);

  kinfo("gdt: loaded 5 flat descriptors (ring 0 + ring 3)");
}
