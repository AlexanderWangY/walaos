/* arch/i386/interrupt.S */

.globl isr0_stub
.globl isr14_stub

.extern interrupt_handler

/* Divide Error
 * No automatic CPU error code.
 */
isr0_stub:
    cli
    pushl $0          /* fake error code */
    pushl $0          /* interrupt number */
    jmp isr_common_stub

/* Page Fault
 * CPU already pushed an error code.
 */
isr14_stub:
    cli
    pushl $14         /* interrupt number */
    jmp isr_common_stub


isr_common_stub:
    pusha

    /*
     * Save segment registers as 32-bit values so the C struct
     * layout using uint32_t fields matches cleanly.
     */
    xorl %eax, %eax
    movw %ds, %ax
    pushl %eax

    xorl %eax, %eax
    movw %es, %ax
    pushl %eax

    xorl %eax, %eax
    movw %fs, %ax
    pushl %eax

    xorl %eax, %eax
    movw %gs, %ax
    pushl %eax

    /*
     * Load kernel data segment selector.
     * Assuming your GDT uses 0x10 for kernel data.
     */
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    /*
     * Pass struct interrupt_frame * to C handler.
     */
    pushl %esp
    call interrupt_handler
    addl $4, %esp

    /*
     * Restore segment registers.
     */
    popl %eax
    movw %ax, %gs

    popl %eax
    movw %ax, %fs

    popl %eax
    movw %ax, %es

    popl %eax
    movw %ax, %ds

    popa

    /*
     * Remove int_no and err_code.
     */
    addl $8, %esp

    iret
