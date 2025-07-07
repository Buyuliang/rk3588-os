#include "printf.h"

// putc 在汇编中定义
extern void putc(char c);

void puts(const char *s) {
    while (*s) {
        if (*s == '\n') {
            putc('\r');
        }
        putc(*s++);
    }
}

void print_decimal(int value) {
    char buf[12];
    int i = 0, negative = 0;
    if (value < 0) {
        negative = 1;
        value = -value;
    }
    do {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);
    if (negative) buf[i++] = '-';
    while (--i >= 0) putc(buf[i]);
}

void print_hex(unsigned int value) {
    const char *hex = "0123456789abcdef";
    for (int i = 28; i >= 0; i -= 4)
        putc(hex[(value >> i) & 0xF]);
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 's': puts(va_arg(args, char *)); break;
                case 'd': print_decimal(va_arg(args, int)); break;
                case 'x': print_hex(va_arg(args, unsigned int)); break;
                case 'c': putc((char)va_arg(args, int)); break;
                case '%': putc('%'); break;
                default: putc('?'); break;
            }
        } else {
            putc(*fmt);
        }
        fmt++;
    }
    va_end(args);
}
