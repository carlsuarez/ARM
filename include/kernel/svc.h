#ifndef SVC_H
#define SVC_H

#include <stdint.h>
#include "drivers/uart.h"
#include "fs/fat/fat32.h"
#include "kernel/task/task.h"

#ifndef SYS_EXIT
#define SYS_EXIT 1
#endif

#ifndef SYS_PRINTF
#define SYS_PRINTF 2
#endif

#ifndef SYS_READ
#define SYS_READ 3
#endif

typedef struct regs
{
    int32_t r0, r1, r2, r3;
    int32_t r4, r5, r6, r7;
    int32_t r8, r9, r10, r11, r12;
    uint32_t lr;
    uint32_t spsr;
} regs_t;

void svc_handler_c(regs_t *regs);

#endif