ENTRY(_start)

. = 0x80000000;

SECTIONS {
    .text : ALIGN(4K) {
        *(.text);
    }

    .bss : ALIGN(4K) {
        PROVIDE(bss_start = .);
        *(.bss);
        PROVIDE(bss_end = .);
    }

    .stack : ALIGN(4K) {
        . += 16K;
        PROVIDE(stack_top = .);
    }

    .rodata : ALIGN(4K) {
        *(.rodata);
    }

    .data : ALIGN(4K) {
        *(.data);
    }
}
