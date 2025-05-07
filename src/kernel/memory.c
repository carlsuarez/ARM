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