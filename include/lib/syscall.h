#ifndef SYSCALL_H
#define SYSCALL_H
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include "kernel/task.h"
#include "lib/printf.h"

#define SYS_EXIT 1
#define SYS_PRINTF 2
#define SYS_READ 3

typedef struct regs
{
    int32_t r0, r1, r2, r3;
    int32_t r4, r5, r6, r7;
    int32_t r8, r9, r10, r11, r12;
    uint32_t lr;
    uint32_t spsr;
} regs_t;

int32_t syscall(int32_t num, int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3);
int32_t svc_handler_c(regs_t *regs);

#endif