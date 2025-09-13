#include <common/string.h>
#include <kernel/lib/malloc.h>

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
    char *buffer = (char *)kmalloc(buffer_size);
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
            char *new_buffer = (char *)kmalloc(buffer_size);
            if (!new_buffer)
            {
                kfree(buffer);
                return NULL;
            }

            // Copy existing content
            for (size_t i = 0; i < current_pos; i++)
            {
                new_buffer[i] = buffer[i];
            }

            kfree(buffer);
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
        char *final_buffer = (char *)kmalloc(final_size);
        if (final_buffer)
        {
            for (size_t i = 0; i < final_size; i++)
            {
                final_buffer[i] = buffer[i];
            }
            kfree(buffer);
            return final_buffer;
        }
    }

    return buffer;
}