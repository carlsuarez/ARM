#ifndef USER_MALLOC_H
#define USER_MALLOC_H

#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);

#endif