#ifndef PRINTK_H
#define PRINTK_H

#include <stdint.h>
#include <kernel/lib/malloc.h>
#include <kernel/drivers/uart.h>
#include <common/string.h>

int32_t printk(const char *fmt, ...);

#endif