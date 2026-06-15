#include <kernel/tty.h>
#include <kernel/keyboard.h>
#include <kernel/log.h>

void kernel_main(void) {
  terminal_initialize();
  keyboard_initialize();
  debug_init();

  debug_write("Starting walaos kernel.\n");

  for (;;) {
    char c = keyboard_poll();

    if (c) {
      debug_write("Writing a character.\n");
      terminal_putchar(c);
    }
  }
}
