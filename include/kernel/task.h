#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "drivers/uart.h"
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
__attribute__((noreturn)) void task_exit(void);
void scheduler(void);

#endif // TASK_H