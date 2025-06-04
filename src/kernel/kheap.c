#include "kernel/kheap.h"

// One linked list for each order
static struct free_block *free_lists[MAX_LEVELS];

struct block_header
{
    uint8_t order;
};

void kheap_init(uintptr_t start)
{
    // Initially, the entire memory is one free block
    free_lists[MAX_LEVELS - 1] = (struct free_block *)start;
    free_lists[MAX_LEVELS - 1]->next = NULL;
}

void *kmalloc(size_t size)
{
    if (size < MIN_BLOCK_SIZE)
        size = MIN_BLOCK_SIZE;

    // Add space for header
    size += sizeof(struct block_header);

    size_t k = 0;
    size_t block_size = MIN_BLOCK_SIZE;
    while (block_size < size)
    {
        block_size <<= 1;
        k++;
    }

    // Find the first non-empty free list >= k
    size_t level = k;
    while (level < MAX_LEVELS && free_lists[level] == NULL)
        level++;

    if (level >= MAX_LEVELS)
        return NULL; // Out of memory

    // Split blocks until we reach the desired level
    while (level > k)
    {
        // Pop head of linked list
        struct free_block *block = free_lists[level];
        free_lists[level] = block->next;

        // Split into two buddies
        size_t buddy_size = (1 << (level - 1 + 5));                                      // + 5 because min is 32
        struct free_block *buddy = (struct free_block *)((uintptr_t)block + buddy_size); // Calculate address of buddy

        /* Add both buddies to the lower level */
        //* Put next point in free block (not being used so no problems!)
        buddy->next = free_lists[level - 1];
        free_lists[level - 1] = buddy;
        block->next = free_lists[level - 1];
        free_lists[level - 1] = block;

        level--;
    }

    // Take the first block from the free list
    struct free_block *allocated_block = free_lists[level];
    free_lists[level] = allocated_block->next;

    // Store order in header
    struct block_header *header = (struct block_header *)allocated_block;
    header->order = (uint8_t)level;

    return (void *)(header + 1); // Return memory after header
}

void kfree(void *ptr)
{
    if (ptr == NULL)
        return;

    struct block_header *header = ((struct block_header *)ptr) - 1;
    size_t k = header->order;
    size_t block_size = MIN_BLOCK_SIZE << k;

    struct free_block *block = (struct free_block *)header;
    block->next = free_lists[k];
    free_lists[k] = block;

    // Try to merge with buddy
    while (k < MAX_LEVELS - 1)
    {
        struct free_block *buddy = (struct free_block *)((uintptr_t)block ^ block_size); // Caclculate address of buddy
        struct free_block *prev = NULL;
        struct free_block *curr = free_lists[k];

        // Search for buddy in free list
        while (curr != NULL && curr != buddy)
        {
            prev = curr;
            curr = curr->next;
        }

        if (curr != buddy)
            break; // Buddy not free, can't merge

        // Remove buddy from free list
        if (prev == NULL)
            free_lists[k] = buddy->next;
        else
            prev->next = buddy->next;

        // Merge block and buddy into a larger block
        if (block > buddy)
            block = buddy;

        // Move to the next level
        k++;
        block_size <<= 1;
        block->next = free_lists[k];
        free_lists[k] = block;
    }
}

void *krealloc(void *ptr, size_t new_size)
{
    if (!ptr)
        return kmalloc(new_size); // realloc(NULL, size) is malloc

    size_t old_size = 1 << ((struct block_header *)ptr - 1)->order;
    if (new_size == 0)
    {
        kfree(ptr);
        return NULL;
    }

    // If the new size fits in the current allocation, keep it
    size_t block_size = MIN_BLOCK_SIZE;
    while (block_size < old_size)
        block_size <<= 1;

    if (new_size <= block_size)
        return ptr;

    // Allocate new block
    void *new_ptr = kmalloc(new_size);
    if (!new_ptr)
        return NULL; // allocation failed

    // Copy old data to new block
    memcpy(new_ptr, ptr, old_size);

    // Free old block
    kfree(ptr);

    return new_ptr;
}
