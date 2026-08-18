// kernel main entry point, but uses architecture compiled from arch/

// From a0, and a1 respectively, DO NOT TOUCH a0 OR a1 in entry.s. DO NOT DO NOT DO NOT
#include <console.h>
#include <platform.h>
#include <stdint.h>
void kmain(unsigned long hart_id, unsigned long dtb) {
    (void)hart_id;
    (void)dtb;

    platform_init();
    console_puts("walaOS: hello from ");
    console_puts(platform_name());
    console_puts("\n");

    // Poll for characters
    int o;
    while ((o = platform_console_getc()) != -1) {
        console_putc(o);
    }

    for (;;)
        __asm__ volatile ("wfi");
}
