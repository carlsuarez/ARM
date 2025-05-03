#include "lib/printf.h"

static void append_char(char **buf, size_t *remaining, char c)
{
    if (*remaining > 1) // Need space for null terminator
    {
        **buf = c;
        (*buf)++;
        (*remaining)--;
    }
}

static void append_string(char **buf, size_t *remaining, const char *s)
{
    if (!s)
        s = "(null)";
    while (*s && *remaining > 1)
    {
        append_char(buf, remaining, *s++);
    }
}

static void append_number(char **buf, size_t *remaining, int32_t val, int base, bool is_signed)
{
    char temp[32];
    char *t = temp + sizeof(temp) - 1;
    *t = '\0';

    bool is_neg = is_signed && (val < 0);
    uint32_t uval = is_neg ? -val : val;

    const char *digits = "0123456789ABCDEF";

    do
    {
        *--t = digits[uval % base];
        uval /= base;
    } while (uval);

    if (is_neg)
    {
        *--t = '-';
    }

    append_string(buf, remaining, t);
}

int32_t vsnprintf(char *str, size_t size, const char *fmt, va_list args)
{
    if (!str || size == 0)
        return 0;

    char *buf = str;
    size_t remaining = size;

    for (const char *p = fmt; *p && remaining > 1; p++)
    {
        if (*p != '%')
        {
            append_char(&buf, &remaining, *p);
            continue;
        }

        p++;
        if (!*p)
            break; // Handle trailing %

        switch (*p)
        {
        case 'd':
            append_number(&buf, &remaining, va_arg(args, int32_t), 10, true);
            break;

        case 'u':
            append_number(&buf, &remaining, va_arg(args, uint32_t), 10, false);
            break;

        case 'x':
            append_number(&buf, &remaining, va_arg(args, uint32_t), 16, false);
            break;

        case 's':
            append_string(&buf, &remaining, va_arg(args, const char *));
            break;

        case 'c':
            append_char(&buf, &remaining, (char)va_arg(args, int));
            break;

        case '%':
            append_char(&buf, &remaining, '%');
            break;

        case 'f':
            // Placeholder for floating point
            append_string(&buf, &remaining, "(floating-point not supported)");
            break;

        default:
            // Output the unsupported specifier
            append_char(&buf, &remaining, '%');
            append_char(&buf, &remaining, *p);
            break;
        }
    }

    // Ensure null termination
    *buf = '\0';
    return buf - str;
}

int32_t snprintf(char *str, size_t size, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int32_t len = vsnprintf(str, size, fmt, args);
    va_end(args);
    return len;
}

void printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);

    syscall(SYS_PRINTF, (int32_t)buf, 0, 0, 0);

    va_end(args);
}
