#pragma once

#include <arch/riscv64/trap.h>
#include <stdint.h>

enum thread_state {
  RUNNING,
  READY,
  NEW,
};

struct thread {
  uint64_t id;
  void *stack;
  struct trap_frame *tf;
  enum thread_state state;
  struct thread *next;
};

void sched_init(void);
void sched_start(void);
struct thread *thread_create(void (*entry)(void *), void *args);
void thread_start(struct thread *t);
struct trap_frame *sched_tick(struct trap_frame *tf);
