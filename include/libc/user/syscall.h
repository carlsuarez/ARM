#ifndef SYSCALL_H
#define SYSCALL_H
#include <stdint.h>
#include <stddef.h>

#ifndef SYS_EXIT
#define SYS_EXIT 1
#endif

#ifndef SYS_PRINTF
#define SYS_PRINTF 2
#endif

#ifndef SYS_READ
#define SYS_READ 3
#endif

int32_t syscall(int32_t num, int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3);

#endif