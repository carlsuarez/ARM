#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>

void *malloc(uint32_t size);
void free(void *ptr);
void memset(void *ptr, uint32_t value, uint32_t num);

#endif