.global _start
_start:
    csrw satp, x0
    la sp, stack_top

    la t0, bss_start
    la t1, bss_end
clear_bss:
    bgeu t0, t1, enter_supervisor
    sb x0, 0(t0)
    addi t0, t0, 1
    j clear_bss
enter_supervisor:
    csrr t0, mstatus # Load machine status
    li t1, ~(3 << 11) # Create clearing bitmask for MPP
    and t0, t0, t1 # Clear MPP from t0
    li t1, (1 << 11) # Load mask for supervisor mode MPP = 01
    or t0, t0, t1 # Write MPP into t0
    csrw mstatus, t0 # Store modified t0 back to mstatus
    la t0, kmain
    csrw mepc, t0
    mret
hang:
    wfi
    j hang
