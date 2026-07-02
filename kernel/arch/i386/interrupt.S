/* arch/i386/interrupt.S */

.macro ISR_NOERR num
 .globl isr\num\()_stub
isr\num\()_stub:
 cli
 pushl $0
 pushl $\num
 jmp common_stub
.endm


.macro ISR_ERR num
 .globl isr\num\()_stub
isr\num\()_stub:
 cli
 pushl $\num
 jmp common_stub
.endm

.macro IRQ num
 .globl irq\num\()_stub
irq\num\()_stub:
 cli
 pushl $0
 pushl $(\num + 32)
 jmp common_stub
.endm

.extern interrupt_handler

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR 8
ISR_NOERR 9
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR 17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

IRQ 0
IRQ 1
IRQ 2
IRQ 3
IRQ 4
IRQ 5
IRQ 6
IRQ 7
IRQ 8
IRQ 9
IRQ 10
IRQ 11
IRQ 12
IRQ 13
IRQ 14
IRQ 15

common_stub:
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
