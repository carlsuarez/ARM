#ifndef LIB_PRINTF_H
#define LIB_PRINTF_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "lib/syscall.h"

int32_t vsnprintf(int8_t *str, size_t size, const int8_t *fmt, va_list args);
int32_t snprintf(int8_t *str, size_t size, const int8_t *fmt, ...);
void printf(const int8_t *fmt, ...);

#endif