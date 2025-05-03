#include "lib/task.h"

__attribute__((noreturn)) void exit(void)
{
    syscall(SYS_EXIT, 0, 0, 0, 0); // Exit status = 0
}