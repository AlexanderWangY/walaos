#include <kernel/panic.h>
#include <stdio.h>

void panic(const char* message) {
  printf("\n");
  printf("[PANIC] %s\n", message);
  printf("Goodbye cruel world.\n");

  __asm__ volatile ("cli");

  for (;;) {
    __asm__ volatile ("hlt");
  }
}
