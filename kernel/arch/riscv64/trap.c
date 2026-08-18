#include <stdint.h>
#include <trap.h>

extern void supervisor_trap_entry(void);

static inline void write_stvec(uintptr_t value) {
  __asm__ volatile ("csrw stvec, %0" :: "r"(value));
}

void trap_init(void) {
  write_stvec((uintptr_t)supervisor_trap_entry);
}

void supervisor_interrupt_handler(struct trap_frame *tf) {
  //TODO: parse sscause and sval and figure out what kind of interrupt this is
  (void)tf;
}
