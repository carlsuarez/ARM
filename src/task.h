#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "uart.h"
#include "interrupt.h"

#define MAX_TASKS 4
#define STACK_SIZE 256

struct task
{
    uint32_t *sp;
    uint8_t active;
    uint32_t stack[STACK_SIZE];
};

extern void context_switch(uint32_t **old_sp, uint32_t **new_sp);
extern void task_exit_trampoline(void);

void task_create(void (*entry)(void));
void task_exit();
void scheduler();

#endif // TASK_H