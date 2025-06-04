#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include <stdint.h>
#include "kernel/printk.h"
#include "kernel/interrupt.h"
#include "kernel/memory.h"

#define MAX_TASKS 4
#define STACK_SIZE 256

typedef enum task_state
{
    RUNNING,
    READY,
    BLOCKED,
    TERMINATED
} task_state_t;

struct task
{
    uint32_t *sp;
    task_state_t state;
    uint32_t stack[STACK_SIZE];
};

extern struct task *current;

void task_init(void);
void task_create(void (*entry)(void));
__attribute__((noreturn)) void task_exit(int32_t status);
void scheduler(void);

#endif // KERNEL_TASK_H