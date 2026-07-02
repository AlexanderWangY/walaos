#include "arch/io.h"
#include "kernel/log.h"
#include <arch/idt.h>
#include <stdint.h>

#define IDT_ENTRIES 256
#define IDT_KERNEL_INTERRUPT_GATE 0x8E
#define IDT_KERNEL_TRAP_GATE 0x8F
#define IDT_USER_INTERRUPT_GATE 0xEE
#define IDT_USER_TRAP_GATE 0xEF
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10

// For PIC
#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_CMD PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_CMD PIC2
#define PIC2_DATA (PIC2 + 1)
#define PIC_EOI 0x20

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define CASCADE_IRQ 2
#define IRQ_BASE 32

// Macros for isr stub generation
#define ISR_TABLE(X) \
  X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) \
  X(8) X(9) X(10) X(11) X(12) X(13) X(14) X(15) \
  X(16) X(17) X(18) X(19) X(20) X(21) X(22) X(23) \
  X(24) X(25) X(26) X(27) X(28) X(29) X(30) X(31)

// Macros for irq stub generation (hardware IRQs 0-15, vectors 32-47)
#define IRQ_TABLE(X) \
  X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) \
  X(8) X(9) X(10) X(11) X(12) X(13) X(14) X(15)

// Declarations are legal at file scope, so the extern block is fine here.
#define AS_ISR_EXTERN(n) extern void isr##n##_stub(void);
ISR_TABLE(AS_ISR_EXTERN)
#undef AS_ISR_EXTERN

#define AS_IRQ_EXTERN(n) extern void irq##n##_stub(void);
IRQ_TABLE(AS_IRQ_EXTERN)
#undef AS_IRQ_EXTERN

static void pic_sendeoi(uint8_t irq) {
  if (irq >= 8) {
    outb(PIC2_CMD, PIC_EOI);
  }

  outb(PIC1_CMD, PIC_EOI);
}

static void pic_remap(int master_offset, int sub_offset) {
  outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC1_DATA, master_offset);
  io_wait();
  outb(PIC2_DATA, sub_offset);
  io_wait();
  outb(PIC1_DATA, 1 << CASCADE_IRQ);
  io_wait();
  outb(PIC2_DATA, 2);
  io_wait();

  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  // Mask all IRQs until handlers exist; unmask per-device as drivers land.
  outb(PIC1_DATA, 0xFF);
  outb(PIC2_DATA, 0xFF);
}

struct interrupt_frame {
    uint32_t gs, fs, es, ds;

    uint32_t edi, esi, ebp, esp_dummy;
    uint32_t ebx, edx, ecx, eax;

    uint32_t int_no;
    uint32_t err_code;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};

struct idt_entry {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t zero;
  uint8_t attributes;
  uint16_t offset_high;
} __attribute__((packed));

struct idtr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

static void idt_set_gate(
  struct idt_entry *idt,
  int index,
  void (*handler)(void),
  uint16_t selector,
  uint8_t attributes
) {
  uintptr_t addr = (uintptr_t)handler;
  
  idt[index].offset_low = addr & 0xFFFF;
  idt[index].selector = selector;
  idt[index].zero = 0;
  idt[index].attributes = attributes;
  idt[index].offset_high = (addr >> 16) & 0xFFFF;
}

extern void idt_flush(uint32_t idtr_address);

static struct idt_entry idt[IDT_ENTRIES];
static struct idtr idt_ptr;

static void idt_load() {
  __asm__ volatile("lidt (%0)" : : "m"(idt_ptr) : "memory");
}

static void halt_with_warning(const char *message) __attribute__((noreturn));

static void pic_clear_mask(uint8_t irq) {
  uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
  outb(port, inb(port) & ~(1 << (irq & 7)));
}
static void halt_with_warning(const char *message) {
  kwarn("%s", message);

  for (;;) {
    __asm__ volatile("hlt");
  }
}

static uint32_t pit_count = 0;
static uint32_t pit_seconds = 0;

void init_idt(void) {

  // Generate isr gates
  #define AS_ISR_GATE(n) idt_set_gate(idt, n, isr##n##_stub, KERNEL_CS, IDT_KERNEL_INTERRUPT_GATE);
  ISR_TABLE(AS_ISR_GATE)
  #undef AS_ISR_GATE

  // Generate irq gates at vectors 32-47
  #define AS_IRQ_GATE(n) idt_set_gate(idt, (n) + IRQ_BASE, irq##n##_stub, KERNEL_CS, IDT_KERNEL_INTERRUPT_GATE);
  IRQ_TABLE(AS_IRQ_GATE)
  #undef AS_IRQ_GATE

  idt_ptr.limit = sizeof(idt) - 1;
  idt_ptr.base = (uint32_t)idt;

  idt_load();

  kinfo("idt: loaded descriptors into IDT");
  pic_remap(0x20, 0x28); // Remap to 32 - 47
  kinfo("pic: remapped PIC for irq");

  pic_clear_mask(0);
  kinfo("irq 0: detected and loaded");

  __asm__ volatile("sti");
}

static void irq_interrupt_handler(struct interrupt_frame *frame) {
  uint8_t irq = frame->int_no - 32;

  switch (irq) {
  case 0:
    // PIT TIMER
    if (++pit_count == 18) {
      pit_count = 0;
      debug_printf("PIT Tick: %d\n", ++pit_seconds);
    }
  case 1:
    // stash, this is a 
    break;
  default:
    break;
  }

  pic_sendeoi(irq);
}

void interrupt_handler(struct interrupt_frame *frame) {
  const char *message;

  if (frame->int_no >= 32 && frame->int_no <= 47) {
    irq_interrupt_handler(frame);
    return;
  }

  switch (frame->int_no) {
  case 0:
    message = "isr0: attempt to divide by 0";
    break;
  case 1:
    message = "isr1: debug exception";
    break;
  case 2:
    message = "isr2: non-maskable interrupt";
    break;
  case 3:
    message = "isr3: breakpoint";
    break;
  case 4:
    message = "isr4: overflow";
    break;
  case 5:
    message = "isr5: bound range exceeded";
    break;
  case 6:
    message = "isr6: invalid opcode";
    break;
  case 7:
    message = "isr7: device not available";
    break;
  case 8:
    message = "isr8: double fault";
    break;
  case 9:
    message = "isr9: coprocessor segment overrun";
    break;
  case 10:
    message = "isr10: invalid TSS";
    break;
  case 11:
    message = "isr11: segment not present";
    break;
  case 12:
    message = "isr12: stack-segment fault";
    break;
  case 13:
    message = "isr13: general protection fault";
    break;
  case 14:
    message = "isr14: memory paging fault";
    break;
  case 15:
    message = "isr15: reserved exception";
    break;
  case 16:
    message = "isr16: x87 floating-point exception";
    break;
  case 17:
    message = "isr17: alignment check";
    break;
  case 18:
    message = "isr18: machine check";
    break;
  case 19:
    message = "isr19: SIMD floating-point exception";
    break;
  case 20:
    message = "isr20: virtualization exception";
    break;
  case 21:
    message = "isr21: control protection exception";
    break;
  case 22:
    message = "isr22: reserved exception";
    break;
  case 23:
    message = "isr23: reserved exception";
    break;
  case 24:
    message = "isr24: reserved exception";
    break;
  case 25:
    message = "isr25: reserved exception";
    break;
  case 26:
    message = "isr26: reserved exception";
    break;
  case 27:
    message = "isr27: reserved exception";
    break;
  case 28:
    message = "isr28: hypervisor injection exception";
    break;
  case 29:
    message = "isr29: VMM communication exception";
    break;
  case 30:
    message = "isr30: security exception";
    break;
  case 31:
    message = "isr31: reserved exception";
    break;
  default:
    kwarn("interrupt: unhandled vector %u", frame->int_no);
    return;
  }

  halt_with_warning(message);
}
