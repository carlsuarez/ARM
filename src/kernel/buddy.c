#include "kernel/buddy.h"
#include "kernel/memory.h"

void buddy_init()
{
    uintptr_t heap_start = (uintptr_t)&_buddy_pool_start;

    // Initially, the entire memory is one free block
    free_lists[MAX_LEVELS - 1] = (struct free_block *)heap_start;
    free_lists[MAX_LEVELS - 1]->next = NULL;
}

void *buddy_alloc(size_t size)
{
    if (size < MIN_BLOCK_SIZE)
        size = MIN_BLOCK_SIZE;

    // Find the smallest k such that (1 << k) >= size
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

    return (void *)allocated_block;
}

void buddy_free(void *ptr, size_t size)
{
    if (ptr == NULL)
        return;

    // Find the correct level
    size_t k = 0;
    size_t block_size = MIN_BLOCK_SIZE;
    while (block_size < size)
    {
        block_size <<= 1;
        k++;
    }

    // Push ptr onto linked list at level k
    struct free_block *block = (struct free_block *)ptr;
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