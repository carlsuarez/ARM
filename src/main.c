#include "uart.h"

void main(void)
{
    struct pl011_t uart;
    uart_init(&uart, PL011_BASE, PL011_CLOCK);
    uart_puts(&uart, "Hello, World!\n");
    while (1)
        ;
}