#include "uart.h"

volatile uint32_t *get_uart_reg(const struct pl011_t *dev, uint32_t offset)
{
    const uint64_t addr = dev->base + offset;

    return (volatile uint32_t *)(uintptr_t)(addr);
}

static void calculate_divisiors(const struct pl011_t *dev, uint32_t *integer, uint32_t *fractional)
{
    // Want: div = 4 * F_UARTCLK / baud_rate;
    // Use 64-bit math to avoid overflow and do fixed-point math.

    uint64_t temp = ((uint64_t)4 * dev->clock << 6);  // Multiply by 64 (shift left 6) to preserve fractional part
    uint32_t div = (uint32_t)(temp / dev->baud_rate); // Result is fixed-point: 6 fractional bits

    *fractional = div & 0x3f;       // Lower 6 bits = fractional part
    *integer = (div >> 6) & 0xffff; // Upper bits = integer part
}

static void wait_tx_complete(const struct pl011_t *dev)
{
    while (*get_uart_reg(dev, UART_FR) & UART_FR_BUSY)
        ;
}

int uart_init(struct pl011_t *dev, uint64_t base, uint64_t clock)
{
    dev->base = base;
    dev->clock = clock;
    dev->baud_rate = 115200;
    dev->data_bits = 8;
    dev->stop_bits = 1;
    dev->parity = 0;

    return pl011_reset(dev);
}

int pl011_reset(const struct pl011_t *dev)
{
    uint32_t cr = *get_uart_reg(dev, UART_CR);
    uint32_t lcr = *get_uart_reg(dev, UART_LCRH);
    uint32_t ibrd, fbrd;

    // Disable UART before anything else
    *get_uart_reg(dev, UART_CR) = (cr & ~UART_CR_UARTEN);

    // Wait for any ongoing transmissions to complete
    wait_tx_complete(dev);

    // Flush FIFOs
    *get_uart_reg(dev, UART_LCRH) = (lcr & ~UART_LCRH_FEN);

    // Set frequency divisors (UARTIBRD and UARTFBRD) to configure the speed
    calculate_divisiors(dev, &ibrd, &fbrd);
    *get_uart_reg(dev, UART_IBRD) = ibrd;
    *get_uart_reg(dev, UART_FBRD) = fbrd;

    // Set line control register (UARTLCRH) to configure data format
    // 8 data bits, 1 stop bit, no parity
    *get_uart_reg(dev, UART_LCRH) = UART_LCRH_WLEN_8_gc | UART_LCRH_STP2;

    // Mask all interrupts by setting corresponding bits to 1
    *get_uart_reg(dev, UART_IMSC) = 0x7ff;

    // Disable DMA by setting all bits to 0
    *get_uart_reg(dev, UART_DMACR) = 0x0;

    // Enable transmission.
    *get_uart_reg(dev, UART_CR) = UART_CR_TXE;

    // Enable UART
    *get_uart_reg(dev, UART_CR) = UART_CR_UARTEN;

    return 0;
}

void uart_putc(const struct pl011_t *dev, char c)
{
    // Wait until TX FIFO is not full
    wait_tx_complete(dev);

    // Write to data register
    *get_uart_reg(dev, UART_DR) = c;
}

void uart_puts(const struct pl011_t *dev, const char *s)
{
    while (*s)
        uart_putc(dev, *s++);
}