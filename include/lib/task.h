#ifndef LIB_TASK_H
#define LIB_TASK_H

#include "kernel/task.h"
#include "lib/syscall.h"
#include <stdint.h>

__attribute__((noreturn)) void exit(void);

#endif