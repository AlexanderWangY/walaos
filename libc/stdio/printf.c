#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static bool print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}

int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;

		if (*format == 'c') {
			format++;
			char c = (char) va_arg(parameters, int);
			if (!maxrem)
				return -1;
			if (!print(&c, sizeof(c)))
				return -1;
			written++;
		} else if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			if (maxrem < len)
				return -1;
			if (!print(str, len))
				return -1;
			written += len;
		} else if (*format == 'd' || *format == 'i') {
			format++;
			int value = va_arg(parameters, int);
			char buf[12];
			int i = 0;
			unsigned int uval;
			if (value < 0) {
				if (!print("-", 1))
					return -1;
				written++;
				uval = (unsigned int)(-(long)value);
			} else {
				uval = (unsigned int)value;
			}
			if (uval == 0) {
				buf[i++] = '0';
			} else {
				while (uval > 0) {
					buf[i++] = '0' + (uval % 10);
					uval /= 10;
				}
			}
			for (int j = i - 1; j >= 0; j--) {
				if (!print(&buf[j], 1))
					return -1;
				written++;
			}
		} else if (*format == 'u') {
			format++;
			unsigned int value = va_arg(parameters, unsigned int);
			char buf[11];
			int i = 0;
			if (value == 0) {
				buf[i++] = '0';
			} else {
				while (value > 0) {
					buf[i++] = '0' + (value % 10);
					value /= 10;
				}
			}
			for (int j = i - 1; j >= 0; j--) {
				if (!print(&buf[j], 1))
					return -1;
				written++;
			}
		} else if (*format == 'x' || *format == 'X') {
			format++;
			unsigned int value = va_arg(parameters, unsigned int);
			const char* digits = (*(format - 1) == 'x')
				? "0123456789abcdef" : "0123456789ABCDEF";
			char buf[8];
			int i = 0;
			if (value == 0) {
				buf[i++] = '0';
			} else {
				while (value > 0) {
					buf[i++] = digits[value & 0xF];
					value >>= 4;
				}
			}
			for (int j = i - 1; j >= 0; j--) {
				if (!print(&buf[j], 1))
					return -1;
				written++;
			}
		} else if (*format == 'p') {
			format++;
			uintptr_t value = (uintptr_t)(va_arg(parameters, void*));
			if (!print("0x", 2))
				return -1;
			written += 2;
			char buf[8];
			for (int i = 7; i >= 0; i--) {
				buf[i] = "0123456789abcdef"[value & 0xF];
				value >>= 4;
			}
			if (!print(buf, 8))
				return -1;
			written += 8;
		} else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len)
				return -1;
			if (!print(format, len))
				return -1;
			written += len;
			format += len;
		}
	}

	va_end(parameters);
	return written;
}
