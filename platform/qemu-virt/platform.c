#include "pmm.h"
#include "vmm.h"
#include <console.h>
#include <platform.h>
#include <stdint.h>
#include <uart/ns16550.h>
#include <irq/plic.h>


#define RAM_SIZE (8ULL * 1024 * 1024 * 1024)

#define UART0 0x10000000UL
#define UART0_IRQ 10
#define UART0_PRIORITY 1
#define PLIC_BASE 0x0c000000UL
#define PLIC_SIZE 0x00600000UL

#define MHART0_CTX 0
#define SHART0_CTX 1
#define MHART1_CTX 2
#define SHART1_CTX 3
// Add more mharts and sharts as we go
// Haha, shart is a funny word... poop

// Declared in linker.ld
extern char kernel_start[];
extern char kernel_end[];

const char *platform_name(void) {
  return "qemu-virt";
}

void platform_init(void) {
    // Enable supervisor hart 0 plic
    set_log_lvl(INFO);


    // Init pmm but first we calculate values
    uintptr_t start = (uintptr_t)kernel_start;
    uintptr_t end = (uintptr_t)kernel_end;
    uintptr_t ram_end = start + RAM_SIZE;

    klog(INFO, "RAM end 0x%lX\n", ram_end);
    
    init_pmm(end, ram_end, 4096);
    vmm_init();

    vmm_map_page(UART0, UART0, PTE_R | PTE_W);
    vmm_map_range(start, start, end - start, PTE_R | PTE_W | PTE_X);
    vmm_map_range(PLIC_BASE, PLIC_BASE, PLIC_SIZE, PTE_R | PTE_W);
    vmm_map_range(pa_to_va(start), start, ram_end - start, PTE_R | PTE_W);
    
    plic_init(PLIC_BASE, SHART0_CTX);
    plic_enable(PLIC_BASE, SHART0_CTX, UART0_IRQ, UART0_PRIORITY);
    ns16550_init(UART0);

    vmm_enable();
    pmm_use_direct_map();
};

void platform_console_putc(char c) {
    ns16550_putc(UART0, c);
}

int platform_console_getc(void) {
    return ns16550_getc(UART0);
}

uint32_t platform_irq_claim(void) {
    return plic_claim(PLIC_BASE, SHART0_CTX);
}

void platform_irq_complete(uint32_t irq) {
    plic_complete(PLIC_BASE, SHART0_CTX, irq);
}
