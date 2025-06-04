#ifndef SLAB_H
#define SLAB_H

#include <stdint.h>
#include <stddef.h>
#include "kernel/kheap.h"
#include "kernel/page_alloc.h"

typedef struct slab_header
{
    struct slab_header *next;
    void *free_list;
    uint32_t used_count;
} slab_header_t;

typedef struct slab_cache
{
    size_t object_size;
    slab_header_t *slabs;
} slab_cache_t;

slab_cache_t *create_slab_cache(size_t object_size);
void destroy_slab_cache(slab_cache_t *cache);
void *slab_alloc(slab_cache_t *cache);
void slab_free(slab_cache_t *cache, void *obj);

#endif
