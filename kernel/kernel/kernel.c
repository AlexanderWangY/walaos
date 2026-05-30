#include <kernel/tty.h>
#include <kernel/log.h>
#include <kernel/panic.h>

void kernel_main(void) {
  terminal_initialize();

  kerror("Something went wrong");
  panic("Killing kernel");
}
