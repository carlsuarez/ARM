#ifndef LIB_PRINTF_H
#define LIB_PRINTF_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <user/lib/syscall.h>
#include <common/string.h>

int printf(const char *fmt, ...);

#endif