#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "uart.h"
#include "memory.h"

#define MAX_TASKS 4
#define STACK_SIZE 512

struct task
{
    uint32_t *sp;
    uint8_t active;
    uint32_t stack[STACK_SIZE];
};

extern struct task *current;

void task_init(void);
void task_create(void (*entry)(void));
void task_exit(void);
void scheduler(void);

#endif // TASK_H