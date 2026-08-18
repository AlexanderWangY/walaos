ENTRY(_start)

. = 0x80000000;

SECTIONS {
    PROVIDE(kernel_start = .);
    .text : ALIGN(4K) {
        *(.text .text.*);
    }

    .rodata : ALIGN(4K) {
        *(.rodata .rodata.*);
    }

    .data : ALIGN(4K) {
        *(.data .data.*);
        . = ALIGN(8);
        PROVIDE(__global_pointer$ = . + 0x800);
        *(.sdata .sdata.*);
    }

    .bss : ALIGN(8) {
        PROVIDE(bss_start = .);
        *(.sbss .sbss.*);
        *(.bss .bss.*);
        *(COMMON);
        PROVIDE(bss_end = .);
    }

    .stack : ALIGN(4K) {
        PROVIDE(stack_bottom = .);
        . += 16K;
        PROVIDE(stack_top = .);
    }
    PROVIDE(kernel_end = .);
}
