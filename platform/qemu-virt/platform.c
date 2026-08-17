#include "../../kernel/include/platform.h"

const char *platform_name(void) {
  return "qemu-virt";
}

void early_platform_init() {};

void platform_init() {};
