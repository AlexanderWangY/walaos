#include <kernel/tty.h>
#include <kernel/log.h>
#include <kernel/panic.h>
#include <kernel/assert.h>

void kernel_main(void) {
  terminal_initialize();

  kerror("Something went wrong");
  kassert(10 == 11);
}
