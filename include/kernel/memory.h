#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>
#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);
void memset(void *ptr, uint8_t value, size_t num);
void *memcpy(const void *restrict _dest, const void *restrict _src, size_t num_bytes);

#endif