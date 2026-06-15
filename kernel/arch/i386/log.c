#include <kernel/log.h>
#include <arch/serial.h>

void debug_init(void) {
  serial_init();
}

void debug_write(const char *data) {
  serial_write(data);
}
