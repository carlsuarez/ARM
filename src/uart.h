#ifndef UART_H
#define UART_H
#include <stdint.h>
#include "hw/pl011.h"
#include "hw/pic.h"

int uart0_init(uint32_t baud_rate);
int uart1_init(uint32_t baud_rate);
void uart_putc(pl011_t *dev, char c);
void uart_puts(pl011_t *dev, const char *str);

#endif