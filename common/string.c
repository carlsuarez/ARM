#include <common/string.h>

size_t strlen(const char *s)
{
    const char *p = s;
    while (*p)
        p++;
    return p - s;
}

int32_t strcmp(const char *s1, const char *s2)
{
    for (; *s1 && *s2 && (*s1 == *s2); s1++, s2++)
        ;
    return *(const char *)s1 - *(const char *)s2;
}

int32_t strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return 0; // Edge case: zero-length comparison

    while (--n && *s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const char *)s1 - *(const char *)s2;
}

char toupper(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - ('a' - 'A');
    }
    return c;
}

char *strchr(const char *s, int32_t c)
{
    while (*s != '\0')
    {
        if (*s == (char)c)
        {
            return (char *)s;
        }
        s++;
    }
    if ((char)c == '\0')
    {
        return (char *)s;
    }
    return NULL;
}

char *strrchr(const char *s, int32_t c)
{
    const char *last = NULL;
    while (*s != '\0')
    {
        if (*s == (char)c)
        {
            last = s;
        }
        s++;
    }
    // Check for terminating null if c is '\0'
    if ((char)c == '\0')
    {
        return (char *)s;
    }
    return (char *)last;
}

char *strcpy(char *dest, const char *src)
{
    char *original_dest = dest; // Save the original destination pointer

    // Copy each character from src to dest until '\0' is encountered
    while ((*dest++ = *src++) != '\0')
        ;

    return original_dest; // Return the start of the destination string
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *original_dest = dest; // Save the original destination pointer
    size_t i;

    // Copy up to n characters from src to dest
    for (i = 0; i < n && src[i] != '\0'; i++)
    {
        dest[i] = src[i];
    }

    // If src is shorter than n, pad dest with '\0'
    for (; i < n; i++)
    {
        dest[i] = '\0';
    }

    return original_dest; // Return the start of the destination string
}

// Format into user-provided buffer (safer, no allocation)
int32_t snprintf(char *buffer, size_t buffer_size, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int32_t result = vsnprintf(buffer, buffer_size, fmt, args);
    va_end(args);
    return result;
}

// Format into user-provided buffer with va_list
int32_t vsnprintf(char *buffer, size_t buffer_size, const char *fmt, va_list args)
{
    if (!buffer || buffer_size == 0)
        return -1;

    char *buf_ptr = buffer;
    char *buf_end = buffer + buffer_size - 1; // Reserve space for null terminator
    const char *fmt_ptr = fmt;
    int32_t chars_written = 0;

    while (*fmt_ptr && buf_ptr < buf_end)
    {
        if (*fmt_ptr == '%' && *(fmt_ptr + 1))
        {
            fmt_ptr++; // Skip '%'

            // Parse width
            int32_t width = 0;
            while (*fmt_ptr >= '0' && *fmt_ptr <= '9')
            {
                width = width * 10 + (*fmt_ptr - '0');
                fmt_ptr++;
            }

            switch (*fmt_ptr)
            {
            case 'd':
            case 'i':
            {
                int32_t val = va_arg(args, int32_t);
                char temp[32];
                int32_t len = int_to_str(val, temp, 10);

                // Apply width padding
                while (width > len && buf_ptr < buf_end)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                // Copy the number
                char *temp_ptr = temp;
                while (*temp_ptr && buf_ptr < buf_end)
                {
                    *buf_ptr++ = *temp_ptr++;
                    chars_written++;
                }
                break;
            }

            case 'u':
            {
                int32_t val = va_arg(args, int32_t);
                char temp[32];
                int32_t len = uint_to_str(val, temp, 10);

                while (width > len && buf_ptr < buf_end)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                char *temp_ptr = temp;
                while (*temp_ptr && buf_ptr < buf_end)
                {
                    *buf_ptr++ = *temp_ptr++;
                    chars_written++;
                }
                break;
            }

            case 'x':
            case 'X':
            {
                int32_t val = va_arg(args, int32_t);
                char temp[32];
                int32_t len = uint_to_str(val, temp, 16);

                while (width > len && buf_ptr < buf_end)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                char *temp_ptr = temp;
                while (*temp_ptr && buf_ptr < buf_end)
                {
                    *buf_ptr++ = *temp_ptr++;
                    chars_written++;
                }
                break;
            }

            case 'c':
            {
                char val = (char)va_arg(args, int32_t);

                while (width > 1 && buf_ptr < buf_end)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                if (buf_ptr < buf_end)
                {
                    *buf_ptr++ = val;
                    chars_written++;
                }
                break;
            }

            case 's':
            {
                const char *str = va_arg(args, const char *);
                if (!str)
                    str = "(null)";
                size_t len = strlen(str);

                while (width > (int32_t)len && buf_ptr < buf_end)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                while (*str && buf_ptr < buf_end)
                {
                    *buf_ptr++ = *str++;
                    chars_written++;
                }
                break;
            }

            case 'p':
            {
                void *ptr = va_arg(args, void *);
                char temp[32];
                int32_t len = ptr_to_str(ptr, temp);

                while (width > len && buf_ptr < buf_end)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                char *temp_ptr = temp;
                while (*temp_ptr && buf_ptr < buf_end)
                {
                    *buf_ptr++ = *temp_ptr++;
                    chars_written++;
                }
                break;
            }

            case '%':
            {
                if (buf_ptr < buf_end)
                {
                    *buf_ptr++ = '%';
                    chars_written++;
                }
                break;
            }

            default:
            {
                if (buf_ptr < buf_end)
                {
                    *buf_ptr++ = '%';
                    chars_written++;
                }
                if (buf_ptr < buf_end)
                {
                    *buf_ptr++ = *fmt_ptr;
                    chars_written++;
                }
                break;
            }
            }
        }
        else
        {
            // Regular character
            *buf_ptr++ = *fmt_ptr;
            chars_written++;
        }

        fmt_ptr++;
    }

    // Null terminate
    *buf_ptr = '\0';

    return chars_written;
}