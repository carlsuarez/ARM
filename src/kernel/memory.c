#include "kernel/memory.h"

void *malloc(size_t size)
{
    return (void *)0xDEADBEEF; // Placeholder for malloc
}

void free(void *ptr)
{
}

void memset(void *ptr, uint8_t value, size_t num)
{
    uint8_t *p = (uint8_t *)ptr;
    for (size_t i = 0; i < num; i++)
    {
        p[i] = value;
    }
}

void *memcpy(const void *restrict _dest, const void *restrict _src, size_t num_bytes)
{
    uint8_t *src = _src;
    uint8_t *dest = _dest;
    for (size_t i = 0; i < num_bytes; i++)
    {
        dest[i] = src[i];
    }

    return _dest;
}

/**
 * Compare two memory blocks
 * @param s1 Pointer to first memory block
 * @param s2 Pointer to second memory block
 * @param n Number of bytes to compare
 * @return <0 if s1 < s2, 0 if s1 == s2, >0 if s1 > s2
 */
int32_t memcmp(const void *s1, const void *s2, size_t n)
{
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] - p2[i];
        }
    }
    return 0;
}