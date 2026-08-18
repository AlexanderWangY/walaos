#include <console.h>
#include <panic.h>
#include <stdint.h>
#include <trap.h>

extern void supervisor_trap_entry(void);

static inline void log_trap(uint64_t sepc, uint64_t sstatus, uint64_t scause, uint64_t stval) {
  uint64_t is_interrupt = scause >> 63;
  if (is_interrupt)
    klog(INFO, "IRQ: sepc 0x%X sstatus %X scause %X stval %X\n", sepc, sstatus, scause, stval);
  else
    klog(INFO, "EXCPTN: sepc 0x%X sstatus %X scause %X stval %X\n", sepc, sstatus, scause, stval);
}

static inline void write_stvec(uintptr_t value) {
  __asm__ volatile ("csrw stvec, %0" :: "r"(value));
}

void trap_init(void) {
  write_stvec((uintptr_t)supervisor_trap_entry);
  __asm__ volatile ("csrs sstatus, %0" :: "r"(1 << 1));
  __asm__ volatile ("csrs sie, %0" :: "r"(1 << 9));
}


void handle_interrupt(struct trap_frame *tf, uint64_t cause) {
  switch (cause) {
    default:
      panic("unhandled interrupt trap cause");
      break;
  }  
}

void handle_exception(struct trap_frame *tf, uint64_t cause) {
  switch (cause) {
    case EXC_S_ECALL:
      log_trap(tf->sepc, tf->sstatus, tf->scause, tf->stval);
      tf->sepc += 4;
      break;
    default:
      panic("unhandled exception trap cause");
      break;
    
  }  
}

void supervisor_trap_handler(struct trap_frame *tf) {
   uint64_t is_interrupt = tf->scause >> 63;
   uint64_t cause = tf->scause & ~(1ULL << 63);
   if (is_interrupt) {
     return handle_interrupt(tf, cause);
   } else {
     return handle_exception(tf, cause);
   }
}

