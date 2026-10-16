#include <sched.h>
#include <stddef.h>

static struct thread *current = NULL;
static struct thread *head = NULL, *tail = NULL;

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
