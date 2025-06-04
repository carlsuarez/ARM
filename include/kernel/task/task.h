#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include <stdint.h>
#include <stddef.h>
#include "kernel/printk.h"
#include "kernel/interrupt.h"
#include "kernel/memory.h"
#include "kernel/slab.h"
#include "kernel/page_alloc.h"

#define MAX_TASKS 4

typedef enum task_state
{
    RUNNING,
    READY,
    BLOCKED,
    TERMINATED
} task_state_t;

typedef struct task
{
    uint32_t *sp;
    task_state_t state;
    uint32_t pid;
    uintptr_t stack_base;
    uint32_t *coarse_pt;
    struct task *next;
} task_t;

extern task_t *current;

void task_init(void);
void task_create(char *name, uintptr_t entry);
__attribute__((noreturn)) void task_exit(int32_t status);
void scheduler(void);

#endif // KERNEL_TASK_H