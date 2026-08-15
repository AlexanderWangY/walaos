.global _start
_start:
    csrw satp, x0
    la sp, stack_top

    la t0, bss_start
    la t1, bss_end
clear_bss:
    bgeu t0, t1, main
    sb x0, 0(t0)
    addi t0, t0, 1
    j clear_bss
main:
    call kmain
hang:
    j hang
