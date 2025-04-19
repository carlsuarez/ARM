#ifndef UART_H
#define UART_H
#include <stdint.h>
#include "hw/pl011.h"

volatile uint32_t *get_uart_reg(const struct pl011_t *dev, uint32_t offset);
int uart_init(struct pl011_t *dev, uint64_t base, uint64_t clock);
int pl011_reset(const struct pl011_t *dev);
void uart_putc(const struct pl011_t *dev, char c);
void uart_puts(const struct pl011_t *dev, const char *str);

#endif