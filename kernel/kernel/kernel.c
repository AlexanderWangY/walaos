#include <kernel/tty.h>
#include <kernel/keyboard.h>
#include <kernel/log.h>
#include <kernel/arch.h>

void kernel_main(void) {
  // Bring up the console first (VGA + serial) so every subsequent boot
  // step is logged to both the screen and the serial port.
  terminal_initialize();
  debug_init();

  kinfo("walaos kernel starting");

  arch_initialize();

  keyboard_initialize();
  kinfo("keyboard: PS/2 driver ready");

  kinfo("boot complete, entering main loop");

  for (;;) {
    char c = keyboard_poll();

    if (c) {
      terminal_putchar(c);
    }
  }
}
