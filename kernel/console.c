#include <console.h>
#include <platform.h>
#include <stdarg.h>

void console_putc(char c) {
    if (c == '\r') {
        platform_console_putc('\n');
        platform_console_putc('\r');
        return;
    }
    if (c == '\n')
        platform_console_putc('\r');
    platform_console_putc(c);
}

void console_puts(const char *s) {
    while (*s)
        console_putc(*s++);
}

// is_capitalized is true if 1 else 0
void print_hex(unsigned int value, int is_capitalized) {
    char ldigits[] = "0123456789abcdef";
    char udigits[] = "0123456789ABCDEF";

    char buf[10];
    int i = 0;

    if (value == 0) {
        console_putc('0');
        return;
    }

    while (value > 0) {
        buf[i++] = is_capitalized == 1 ? udigits[value % 16] : ldigits[value % 16];
        value /= 16;
    }

    while (i > 0)
        console_putc(buf[--i]);
}

void print_uint(unsigned int value) {
    char buf[10];
    int i = 0;
    if (value == 0) {
        console_putc('0');
        return;
    }
    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0)
        console_putc(buf[--i]);
}

void print_int(int value) {
    if (value < 0) {
        console_putc('-');
        print_uint(-(unsigned int)(value));
    } else {
        print_uint((unsigned int)value);
    }
}

void vkprintf(const char *s, va_list args) {
    while (*s) {
        if (*s == '%') {
            s++;

            switch (*s) {
                case 'd': {
                    int value = va_arg(args, int);
                    print_int(value);
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char *);
                    console_puts(str);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    console_putc(c);
                    break;
                }
                case '%': {
                    console_putc('%');
                    break;
                }
                case 'x': {
                    int value = va_arg(args, int);
                    print_hex(value, 0);
                    break;
                }
                case 'X': {
                    int value = va_arg(args, int);
                    print_hex(value, 1);
                    break;
                }
            }
        } else {
            console_putc(*s);
        }

        s++;
    }
}


void kprintf(const char *s, ...) {
    va_list args;
    va_start(args, s);

    vkprintf(s, args);
    
    va_end(args);
}

void klog(enum LOG_LEVEL lvl, const char *s, ...){
    switch (lvl) {
        case DEBUG:
            console_puts("[DEBUG] ");
            break;
        case INFO:
            console_puts("[INFO] ");
            break;
        case WARN:
            console_puts("[WARN] ");
            break;
        case PANIC:
            console_puts("[PANIC] ");
            break;
    }

    va_list args;
    va_start(args, s);

    vkprintf(s, args);
    
    va_end(args);

}
