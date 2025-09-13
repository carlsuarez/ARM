#include <kernel/arch/arm/svc.h>

void svc_handler_c(regs_t *regs)
{
    switch (regs->r7)
    {
    case SYS_PRINTF:
        uart_puts(uart0, (const char *)regs->r0);
        break;
    case SYS_EXIT:
        task_exit(regs->r0); // noreturn
        break;
    default:
        regs->r0 = (uint32_t)-1; // Unknown syscall
        break;
    }
}