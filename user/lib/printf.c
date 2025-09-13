#include <user/lib/printf.h>

int printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char formatted[1024]; // No malloc yet
    vsnprintf(formatted, 1024, fmt, args);
    va_end(args);

    if (!formatted)
    {
        return -1;
    }

    syscall(SYS_PRINTF, (int32_t)formatted, 0, 0, 0);

    // Calculate length for return value
    int len = strlen(formatted);
    // free(formatted);

    return len;
}