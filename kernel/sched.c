#include "arch/riscv64/trap.h"
#include "kheap.h"
#include <sched.h>
#include <stddef.h>
#include <stdint.h>

#define STACK_SIZE (16 * 1024)

static uint64_t next_id = 0;
static struct thread *current = NULL;
static struct thread *head = NULL, *tail = NULL;

static struct thread init_thread;

static void enqueue(struct thread *t) {
  t->next = NULL;

  if (tail) {
    tail->next = t;
  } else {
    head = t;
  }

  tail = t;
}

static struct thread* dequeue(void) {
  struct thread *t = head;
  if (t) {
    head = head->next;
    if (!head) tail = NULL;
  }

  return t;
}

static void schedule(void) {
  struct thread *next = dequeue();
  if (!next) return;

  if (current) {
    current->state = READY;
    enqueue(current);
  }

  next->state = RUNNING;
  current = next;
}

void sched_init(void) {
  // Initialize boot thread
  init_thread.id = next_id++;
  init_thread.state = RUNNING;
  init_thread.tf = NULL; // this will get saved on the first interrupt
  init_thread.next = NULL;
  init_thread.stack = NULL;
  current = &init_thread;
};

struct thread *thread_create(void (*entry)(void), void *arg) {
  struct thread *t = kmalloc(sizeof *t);
  void *stack = kmalloc(STACK_SIZE);

  uint8_t *top = (uint8_t *)stack + STACK_SIZE;
  struct trap_frame *tf = (struct trap_frame *)(top - sizeof *tf);
  memset
}

struct trap_frame *sched_tick(struct trap_frame *tf) {
  if (!current) return tf;

  // Save current state, cycle, then return new state to restore
  current->tf = tf;
  schedule();
  return current->tf;
}
