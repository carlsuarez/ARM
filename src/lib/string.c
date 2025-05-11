#include "lib/string.h"

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
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
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
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

char toupper(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return c - ('a' - 'A');
    }
    return c;
}

char *strchr(const char *s, int c)
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

char *strrchr(const char *s, int c)
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