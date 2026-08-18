#include <console.h>
#include <panic.h>
#include <stdint.h>
#include <trap.h>

extern void supervisor_trap_entry(void);

static inline void write_stvec(uintptr_t value) {
  __asm__ volatile ("csrw stvec, %0" :: "r"(value));
}

void trap_init(void) {
  write_stvec((uintptr_t)supervisor_trap_entry);
}


void handle_interrupt(struct trap_frame *tf, uint64_t cause) {
  switch (cause) {
    case 0:
      break;
    case 1:
      // supervisor software interrupt
      break;
    case 2:
    case 3:
    case 4:
      break;
    case 5:
      // supervisor timer interrupt
      break;
    case 6:
    case 7:
    case 8:
      break;
    case 9:
      // supervisor external interrupt
      break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      break;
    default:
      // Anything >= 16 is for platform use
      break;
  }  
}

void handle_exception(struct trap_frame *tf, uint64_t cause) {
  switch (cause) {
    case 0:
      // Instruction addr misaligned
      break;
    case 1:
      // Instruction access fault
      break;
    case 2:
      // Illegal instruction
      break;
    case 3:
      // Breakpoint
      break;
    case 4:
      // Load addr misaligned
      break;
    case 5:
      // Load access fault
      break;
    case 6:
      // Store/AMO addr misaligned
      break;
    case 7:
      // Store/AMO access fault
      break;
    case 8:
      break;
    case 9:
      // supervisor external interrupt
      break;
    case 10:
    case 11:
      break;
    case 12:
      // Instruction page fault
      break;
    case 13:
      // Load page fault
      break;
    case 14:
      break;
    case 15:
      // Store/AMO page fault
      break;
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
      break;
    default:
      // Anything 24-31 is for custom use
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

