#include "libc/user/printf.h"

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buf[1024]; // No malloc yet
    char *formatted = vsnprintf(buf, 1024, fmt, args);
    va_end(args);

    if (!formatted)
    {
        return -1;
    }

    syscall(SYS_PRINTF, formatted, 0, 0, 0);

    // Calculate length for return value
    int len = strlen(formatted);
    // free(formatted);

    return len;
}