#include <arch/io.h>
#include <arch/serial.h>
#include <stdint.h>

static int serial_is_empty(void) {
  return inb(COM1 + 5) & 0x20;
}

void serial_init(void) {
  outb(COM1 + 1, 0x00); // Disable interrupts
  outb(COM1 + 3, 0x80); // Enable DLAB


  // Set divisor to 3 (38400 baud)
  // Divisor = 115200 / desired_baud
  outb(COM1 + 0, 0x03);
  outb(COM1 + 1, 0x00);

  outb(COM1 + 3, 0x03);
  outb(COM1 + 2, 0xC7);
  outb(COM1 + 4, 0x0B);
}

void serial_putchar(char c) {
  while (!serial_is_empty()) {
    // BUSY WAIT
  }

  outb(COM1, (uint8_t)c);
}

void serial_write(const char *data) {
  while (*data) {
    if (*data == '\n') {
      serial_putchar('\r');
    }

    serial_putchar(*data++);
  }
}
