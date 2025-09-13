#include <user/lib/syscall.h>

int32_t syscall(int32_t num, int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3)
{
    register int32_t r0 asm("r0") = arg0;
    register int32_t r1 asm("r1") = arg1;
    register int32_t r2 asm("r2") = arg2;
    register int32_t r3 asm("r3") = arg3;
    register int32_t r7 asm("r7") = num;

    asm volatile(
        "svc 0\n"
        : "+r"(r0)
        : "r"(r1), "r"(r2), "r"(r3), "r"(r7)
        : "memory");

    return r0;
}