ENTRY(_start)

. = 0x80000000;

SECTIONS {
    PROVIDE(kernel_start = .);
    .text : ALIGN(4K) {
        *(.text);
    }

    .bss : ALIGN(4K) {
        PROVIDE(bss_start = .);
        *(.bss);
        *(.sbss);
        PROVIDE(bss_end = .);
    }

    .stack : ALIGN(4K) {
        PROVIDE(stack_bottom = .);
        . += 16K;
        PROVIDE(stack_top = .);
    }

    .rodata : ALIGN(4K) {
        *(.rodata .rodata.*);
    }

    .data : ALIGN(4K) {
        *(.data);
        *(.sdata);
    }
    PROVIDE(kernel_end = .);
}
