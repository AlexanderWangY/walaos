#pragma once

#include <arch/riscv64/trap.h>
#include <stdint.h>

enum thread_state {
  RUNNING,
  READY,
  WAITING,
  ZOMBIE,
};

struct thread {
  struct trap_frame *tf;
  enum thread_state state;
  uint64_t id;
  uintptr_t stack_pa;
  struct thread *next;
};


