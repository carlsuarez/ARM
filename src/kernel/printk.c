#include "kernel/printk.h"

int32_t printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buf[256];
    int32_t ret = vsnprintf(buf, sizeof(buf), fmt, args);

    uart_puts(uart0, buf);

    va_end(args);

    return ret;
}