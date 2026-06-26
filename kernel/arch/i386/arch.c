#include <kernel/arch.h>
#include <kernel/log.h>
#include <arch/gdt.h>
#include <arch/idt.h>

void arch_initialize(void) {
  kinfo("arch: initializing i386 platform");
  init_gdt();
  init_idt();
}
