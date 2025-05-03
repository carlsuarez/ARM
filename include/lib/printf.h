#ifndef LIB_PRINTF_H
#define LIB_PRINTF_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "lib/syscall.h"

int32_t vsnprintf(char *str, size_t size, const char *fmt, va_list args);
int32_t snprintf(char *str, size_t size, const char *fmt, ...);
void printf(const char *fmt, ...);

#endif