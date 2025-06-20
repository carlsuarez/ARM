#ifndef PRINTK_H
#define PRINTK_H

#include <stdint.h>
#include "libc/kernel/malloc.h"
#include "drivers/uart.h"
#include "libc/common/string.h"

int32_t printk(const char *fmt, ...);

#endif