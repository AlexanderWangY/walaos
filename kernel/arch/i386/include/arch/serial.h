#ifndef ARCH_I386_SERIAL_H
#define ARCH_I386_SERIAL_H

#define COM1 0x3F8

void serial_init(void);
void serial_putchar(char c);
void serial_write(const char* data);

#endif
