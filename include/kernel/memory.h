#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>
#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);
void memset(void *ptr, uint8_t value, size_t num);
void *memcpy(const void *restrict _dest, const void *restrict _src, size_t num_bytes);

/**
 * Compare two memory blocks
 * @param s1 Pointer to first memory block
 * @param s2 Pointer to second memory block
 * @param n Number of bytes to compare
 * @return <0 if s1 < s2, 0 if s1 == s2, >0 if s1 > s2
 */
int32_t memcmp(const void *restrict s1, const void *restrict s2, size_t n);

#endif