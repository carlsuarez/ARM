#include "libc/common/string.h"

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

char *strdup(const char *s)
{
    if (s == NULL)
    {
        return NULL;
    }

    size_t len = strlen(s) + 1; // +1 for the null terminator
    char *dup = lib_malloc(len);

    if (dup != NULL)
    {
        memcpy(dup, s, len);
    }

    return dup;
}

// Helper function to convert integer to string
static int32_t int_to_str(int32_t value, char *str, int32_t base)
{
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int32_t tmp_value;
    int32_t len = 0;

    // Handle negative numbers for base 10
    if (value < 0 && base == 10)
    {
        *ptr++ = '-';
        value = -value;
        len++;
    }

    // Convert to string (reverse order)
    do
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
        len++;
    } while (value);

    // Null terminate
    *ptr-- = '\0';

    // Reverse the string (skip sign if present)
    char *start = (*str == '-') ? str + 1 : str;
    while (start < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *start;
        *start++ = tmp_char;
    }

    return len;
}

// Helper function to convert integer to string
static int32_t uint_to_str(int32_t value, char *str, int32_t base)
{
    char *ptr = str;
    char *ptr1 = str;
    char tmp_char;
    int32_t tmp_value;
    int32_t len = 0;

    // Convert to string (reverse order)
    do
    {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
        len++;
    } while (value);

    // Null terminate
    *ptr-- = '\0';

    // Reverse the string
    while (str < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *str;
        *str++ = tmp_char;
    }

    return len;
}

// Helper function to convert pointer to hex string
static int32_t ptr_to_str(void *ptr, char *str)
{
    uintptr_t value = (uintptr_t)ptr;
    char *p = str;
    int32_t len = 0;

    *p++ = '0';
    *p++ = 'x';
    len += 2;

    // Handle null pointer
    if (value == 0)
    {
        *p++ = '0';
        *p = '\0';
        return len + 1;
    }

    // Convert to hex
    char hex_digits[sizeof(uintptr_t) * 2];
    int32_t hex_len = 0;

    while (value > 0)
    {
        hex_digits[hex_len++] = "0123456789abcdef"[value & 0xF];
        value >>= 4;
    }

    // Reverse and copy
    for (int32_t i = hex_len - 1; i >= 0; i--)
    {
        *p++ = hex_digits[i];
        len++;
    }

    *p = '\0';
    return len;
}

// Core formatting function - returns dynamically allocated string
char *sprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *result = vsprintf(fmt, args);
    va_end(args);
    return result;
}

// Core formatting function with va_list
char *vsprintf(const char *fmt, va_list args)
{
    // Initial buffer size - will grow as needed
    size_t buffer_size = 512;
    char *buffer = (char *)lib_malloc(buffer_size);
    if (!buffer)
    {
        return NULL;
    }

    char *buf_ptr = buffer;
    const char *fmt_ptr = fmt;
    int32_t chars_written = 0;

    while (*fmt_ptr)
    {
        // Ensure we have enough space (reserve 128 bytes for safety)
        if ((buf_ptr - buffer) >= (buffer_size - 128))
        {
            size_t current_pos = buf_ptr - buffer;
            buffer_size *= 2;
            char *new_buffer = (char *)lib_malloc(buffer_size);
            if (!new_buffer)
            {
                lib_free(buffer);
                return NULL;
            }

            // Copy existing content
            for (size_t i = 0; i < current_pos; i++)
            {
                new_buffer[i] = buffer[i];
            }

            lib_free(buffer);
            buffer = new_buffer;
            buf_ptr = buffer + current_pos;
        }

        if (*fmt_ptr == '%' && *(fmt_ptr + 1))
        {
            fmt_ptr++; // Skip '%'

            // Parse width (simple implementation)
            int32_t width = 0;
            while (*fmt_ptr >= '0' && *fmt_ptr <= '9')
            {
                width = width * 10 + (*fmt_ptr - '0');
                fmt_ptr++;
            }

            switch (*fmt_ptr)
            {
            case 'l':
            {
                // Handle long specifiers
                fmt_ptr++;
                if (*fmt_ptr == 'd' || *fmt_ptr == 'i')
                {
                    long val = va_arg(args, long);
                    char temp[64];
                    // For simplicity, cast to int32_t (extend as needed)
                    int32_t len = int_to_str((int32_t)val, temp, 10);

                    // Apply width padding
                    while (width > len)
                    {
                        *buf_ptr++ = ' ';
                        chars_written++;
                        width--;
                    }

                    strcpy(buf_ptr, temp);
                    buf_ptr += len;
                    chars_written += len;
                }
                break;
            }

            case 'd':
            case 'i':
            {
                int32_t val = va_arg(args, int32_t);
                char temp[32];
                int32_t len = int_to_str(val, temp, 10);

                // Apply width padding
                while (width > len)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                strcpy(buf_ptr, temp);
                buf_ptr += len;
                chars_written += len;
                break;
            }

            case 'u':
            {
                int32_t val = va_arg(args, int32_t);
                char temp[32];
                int32_t len = uint_to_str(val, temp, 10);

                while (width > len)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                strcpy(buf_ptr, temp);
                buf_ptr += len;
                chars_written += len;
                break;
            }

            case 'x':
            case 'X':
            {
                int32_t val = va_arg(args, int32_t);
                char temp[32];
                int32_t len = uint_to_str(val, temp, 16);

                while (width > len)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                strcpy(buf_ptr, temp);
                buf_ptr += len;
                chars_written += len;
                break;
            }

            case 'o':
            {
                int32_t val = va_arg(args, int32_t);
                char temp[32];
                int32_t len = uint_to_str(val, temp, 8);

                while (width > len)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                strcpy(buf_ptr, temp);
                buf_ptr += len;
                chars_written += len;
                break;
            }

            case 'c':
            {
                char val = (char)va_arg(args, int32_t);

                while (width > 1)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                *buf_ptr++ = val;
                chars_written++;
                break;
            }

            case 's':
            {
                const char *str = va_arg(args, const char *);
                if (!str)
                    str = "(null)";
                size_t len = strlen(str);

                while (width > (int32_t)len)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                strcpy(buf_ptr, str);
                buf_ptr += len;
                chars_written += len;
                break;
            }

            case 'p':
            {
                void *ptr = va_arg(args, void *);
                char temp[32];
                int32_t len = ptr_to_str(ptr, temp);

                while (width > len)
                {
                    *buf_ptr++ = ' ';
                    chars_written++;
                    width--;
                }

                strcpy(buf_ptr, temp);
                buf_ptr += len;
                chars_written += len;
                break;
            }

            case '%':
            {
                *buf_ptr++ = '%';
                chars_written++;
                break;
            }

            default:
            {
                // Unknown format specifier
                *buf_ptr++ = '%';
                *buf_ptr++ = *fmt_ptr;
                chars_written += 2;
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

    // Resize buffer to exact size needed (optional optimization)
    size_t final_size = (buf_ptr - buffer) + 1;
    if (final_size < buffer_size)
    {
        char *final_buffer = (char *)lib_malloc(final_size);
        if (final_buffer)
        {
            for (size_t i = 0; i < final_size; i++)
            {
                final_buffer[i] = buffer[i];
            }
            lib_free(buffer);
            return final_buffer;
        }
    }

    return buffer;
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