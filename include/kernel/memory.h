#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>

void *malloc(uint32_t size);
void free(void *ptr);
void memset(void *ptr, uint32_t value, uint32_t num);
void *memcpy(const void *restrict _dest, const void *restrict _src, uint64_t num_bytes);

#endif