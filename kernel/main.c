// kernel main entry point, but uses architecture compiled from arch/

// From a0, and a1 respectively, DO NOT TOUCH a0 OR a1 in entry.s. DO NOT DO NOT DO NOT
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

    for (;;)
        __asm__ volatile ("wfi");
}
