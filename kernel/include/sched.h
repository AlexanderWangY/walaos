#pragma once

#include <arch/riscv64/trap.h>
#include <stdint.h>

enum thread_state {
  RUNNING,
  READY,
};

struct thread {
  struct trap_frame *tf;
  enum thread_state state;
  uint64_t id;
  struct thread *next;
};
