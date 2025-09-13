#ifndef TASK_DEFS_H
#define TASK_DEFS_H

#include <defs.h>
#include <kernel/core/task/elf/elf_defs.h>

#define TASK_TEXT_BASE 0x8000000
#define TASK_SO_BASE 0x20000000
#define TASK_STACK_BASE 0x30000000
#define TASK_STACK_SIZE 0x100000 // 1MB stack

typedef struct l1_free_node
{
    uint32_t *l1;
    struct l1_free_node *next;
} l1_free_node_t;

struct PCB
{
    uintptr_t sp;
    enum
    {
        RUNNING,
        READY,
        BLOCKED,
        TERMINATED
    } state;
    uint32_t context[17];
    uint32_t pid;
    int8_t fd;
    uint32_t *pt; // Physical address of L1 page table
    struct
    {
        uintptr_t base_va;
        Elf32_Sym *symtab;
        const char *strtab;
        Elf32_Hash hash;
        uintptr_t next_so_base;
    } elf_info;
    so_entry_task_t *shared_objs;
    char name[11];
    struct PCB *next;
};

#endif