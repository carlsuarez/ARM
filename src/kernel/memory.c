#include "kernel/memory.h"

void *malloc(uint32_t size)
{
    return (void *)0xDEADBEEF; // Placeholder for malloc
}

void free(void *ptr)
{
}

void memset(void *ptr, uint32_t value, uint32_t num)
{
    uint8_t *p = (uint8_t *)ptr;
    for (uint32_t i = 0; i < num; i++)
    {
        p[i] = (uint8_t)value;
    }
}