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

// Macros for isr stub generation
#define ISR_TABLE(X) \
  X(0) X(1) X(2) X(3) X(4) X(5) X(6) X(7) \
  X(8) X(9) X(10) X(11) X(12) X(13) X(14) X(15) \
  X(16) X(17) X(18) X(19) X(20) X(21) X(22) X(23) \
  X(24) X(25) X(26) X(27) X(28) X(29) X(30) X(31)

// Declarations are legal at file scope, so the extern block is fine here.
#define AS_ISR_EXTERN(n) extern void isr##n##_stub(void);
ISR_TABLE(AS_ISR_EXTERN)
#undef AS_ISR_EXTERN

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

static void halt_with_warning(const char *message) {
  kwarn("%s", message);

  for (;;) {
    __asm__ volatile("hlt");
  }
}

void init_idt(void) {

  // Generate isr gates
  #define AS_ISR_GATE(n) idt_set_gate(idt, n, isr##n##_stub, KERNEL_CS, IDT_KERNEL_INTERRUPT_GATE);
  ISR_TABLE(AS_ISR_GATE)
  #undef AS_ISR_GATE

  idt_ptr.limit = sizeof(idt) - 1;
  idt_ptr.base = (uint32_t)idt;

  idt_load();

  kinfo("idt: loaded descriptors into IDT");
}

void interrupt_handler(struct interrupt_frame *frame) {
  const char *message;

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
