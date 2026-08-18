#include <console.h>
#include <panic.h>

void panic(const char *msg) {
  
  console_puts("[KERNEL] panicked: ");
  console_puts(msg);
  console_puts("\n");

  for (;;)
    __asm__ volatile ("wfi");
}
