#include <kernel/lib/printk.h>

int32_t printk(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char formatted[100];
    vsnprintf(formatted, sizeof(formatted), fmt, args);
    va_end(args);

    if (!formatted)
    {
        return -1;
    }

    uart_puts(uart0, formatted);

    // Calculate length for return value
    int len = strlen(formatted);
    // kfree(formatted);

    return len;
}