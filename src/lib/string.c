#include "lib/string.h"

uint64_t strlen(const char *s)
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

int32_t strncmp(const char *s1, const char *s2, uint64_t n)
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