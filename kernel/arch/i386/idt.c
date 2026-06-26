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

extern void isr0_stub(void);
extern void isr14_stub(void);

void init_idt(void) {
  idt_set_gate(idt, 0, isr0_stub, KERNEL_CS, IDT_KERNEL_INTERRUPT_GATE);  
  idt_set_gate(idt, 14, isr14_stub, KERNEL_CS, IDT_KERNEL_INTERRUPT_GATE);

  idt_ptr.limit = sizeof(idt) - 1;
  idt_ptr.base = (uint32_t)idt;

  idt_load();

  kinfo("idt: loaded descriptors into IDT");
}

void interrupt_handler(struct interrupt_frame *frame) {
  if (frame->int_no == 0) {
    kwarn("isr0: attempt to divide by 0");

    for (;;) {
      __asm__ volatile("hlt");
    }
  }

  if (frame->int_no == 14) {
    kwarn("isr14: memory paging fault");

    for (;;) {
      __asm__ volatile("hlt");
    }
  }
}
