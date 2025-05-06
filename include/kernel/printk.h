#ifndef PRINTK_H
#define PRINTK_H

#include <stdint.h>
#include "lib/printf.h"
#include "drivers/uart.h"

int32_t printk(const char *fmt, ...);

#endif