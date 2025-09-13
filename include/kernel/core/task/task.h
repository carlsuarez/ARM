#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/lib/printk.h>
#include <kernel/arch/arm/interrupt.h>
#include <common/memory.h>
#include <kernel/lib/slab.h>
#include <kernel/lib/page_alloc.h>
#include <kernel/core/task/elf/elf.h>
#include <kernel/core/task/task_defs.h>
#include <kernel/arch/arm/mmu.h>

#define MAX_TASKS 4

extern struct PCB *current;

void task_init(void);
int8_t task_create(const char *path, const char *name);
__attribute__((noreturn)) void task_exit(int32_t status);
void scheduler(void);

#endif // KERNEL_TASK_H