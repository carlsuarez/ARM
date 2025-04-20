#ifndef UART_H
#define UART_H
#include <stdint.h>
#include "hw/pl011.h"
#include "hw/pic.h"

static struct pl011_t PLO11_UART0 = {
    .base = UART0_BASE,
    .clock = PL011_CLOCK};

extern struct pl011_t *const uart0;

static struct pl011_t PLO11_UART1 = {
    .base = UART1_BASE,
    .clock = PL011_CLOCK};

extern struct pl011_t *const uart1;

volatile uint32_t *get_uart_reg(const struct pl011_t *dev, uint32_t offset);
int uart0_init(uint32_t baud_rate);
int uart1_init(uint32_t baud_rate);
int pl011_reset(const struct pl011_t *dev);
void uart_putc(const struct pl011_t *dev, char c);
void uart_puts(const struct pl011_t *dev, const char *str);

#endif