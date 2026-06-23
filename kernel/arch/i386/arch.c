#include <kernel/arch.h>
#include <kernel/log.h>
#include <arch/gdt.h>

void arch_initialize(void) {
  kinfo("arch: initializing i386 platform");
  init_gdt();
}
