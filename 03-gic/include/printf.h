#ifndef __PRINTF_H__
#define __PRINTF_H__

#include "stdarg_stub.h"

// putc 在汇编中定义
extern void putc(char c);

void puts(const char *s);
void print_decimal(int value);
void print_hex(unsigned int value);
void printf(const char *fmt, ...);
#endif
