// kernel main entry point, but uses architecture compiled from arch/

// From a0, and a1 respectively, DO NOT TOUCH a0 OR a1 in entry.s. DO NOT DO NOT DO NOT
#include "include/sched.h"
#include "pmm.h"
#include "vmm.h"
#include <kheap.h>
#include <console.h>
#include <platform.h>
#include <stdint.h>
#include <panic.h>


static void counter(void* arg) {
    int id = (int)(intptr_t)arg;
    for (;;) {
        kprintf("Worker %d\n", id);
    }
}

void kmain(unsigned long hart_id, unsigned long dtb) {
    (void)hart_id;
    (void)dtb;

    platform_init();
    console_puts("walaOS: hello from ");
    console_puts(platform_name());
    console_puts("\n");

    __asm__ volatile ("ecall");

    console_puts("Text after ecall\n");
    kprintf("Hello there the answer is %d and I like %s\n", 10, "apples");
    kprintf("64-bit hex test: 0x%lX\n", 0x123456789ABCDEF0UL);

    
    uintptr_t pa = pmm_alloc_page();
    uint64_t *direct = (uint64_t *)pa_to_va(pa);

    direct[0] = 0x123456789ABCDEF0;
    kprintf("direct-map value: 0x%lX\n", direct[0]);

    vmm_map_page(0x60000000, pa, PTE_R | PTE_W);

    // Attempt to allocate kheap
    char *c = kmalloc(10);
    int *nums = kmalloc(20 * sizeof(int));
    char *longg = kmalloc(7453);

    nums[10] = 40;

    kfree(c);
    kfree(nums);
    kfree(longg);

    sched_init();

    struct thread *x = thread_create(counter, (void *)(intptr_t)1);
    struct thread *y = thread_create(counter, (void *)(intptr_t)2);
    struct thread *z = thread_create(counter, (void *)(intptr_t)3);
    thread_start(x);
    thread_start(y);
    thread_start(z);

    sched_start();
    for (;;)
        __asm__ volatile ("wfi");
}
