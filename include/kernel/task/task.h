#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include <stdint.h>
#include <stddef.h>
#include "libc/kernel/printk.h"
#include "kernel/interrupt.h"
#include "libc/common/memory.h"
#include "libc/kernel/slab.h"
#include "libc/kernel/page_alloc.h"

#define MAX_TASKS 4

typedef struct task
{
    uint32_t *sp;
    enum
    {
        RUNNING,
        READY,
        BLOCKED,
        TERMINATED
    } state;
    uint32_t pid;
    uintptr_t stack_base;
    size_t num_pages;
    uintptr_t coarse_pt; // physical address
    uintptr_t va_base;
    char name[11];
    struct task *next;
} task_t;

extern task_t *current;

void task_init(void);
task_t *task_create(uintptr_t entry);
__attribute__((noreturn)) void task_exit(int32_t status);
void scheduler(void);

#endif // KERNEL_TASK_H