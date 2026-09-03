// kernel main entry point, but uses architecture compiled from arch/

// From a0, and a1 respectively, DO NOT TOUCH a0 OR a1 in entry.s. DO NOT DO NOT DO NOT
#include "pmm.h"
#include "vmm.h"
#include <console.h>
#include <platform.h>
#include <stdint.h>
#include <panic.h>
#include <trap.h>

void kmain(unsigned long hart_id, unsigned long dtb) {
    (void)hart_id;
    (void)dtb;

    trap_init();
    platform_init();
    console_puts("walaOS: hello from ");
    console_puts(platform_name());
    console_puts("\n");

    __asm__ volatile ("ecall");

    console_puts("Text after ecall\n");
    kprintf("Hello there the answer is %d and I like %s\n", 10, "apples");
    kprintf("64-bit hex test: 0x%lX\n", 0x123456789ABCDEF0UL);

    
    uintptr_t pa = alloc_pmm();
    uint64_t *direct = (uint64_t *)pa_to_va(pa);

    direct[0] = 0x123456789ABCDEF0;
    kprintf("direct-map value: 0x%lX\n", direct[0]);

    vmm_map_page(0x60000000, pa, PTE_R | PTE_W);
    for (;;)
        __asm__ volatile ("wfi");
}
