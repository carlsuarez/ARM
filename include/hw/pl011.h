#ifndef PL011_H
#define PL011_H
#include <stdint.h>
#include <stddef.h>
#include "defs.h"

typedef struct
{
    volatile uint32_t dr;     // Data Register
    volatile uint32_t rsrecr; // Receive Status/Error Clear Register
    RESERVE_U8(16);           // Reserved
    volatile uint32_t fr;     // Flag Register
    volatile uint32_t _;      // Reserved
    volatile uint32_t ilpr;   // IrDA Low-Power Register
    volatile uint32_t ibrd;   // Integer Baud Rate Register
    volatile uint32_t fbrd;   // Fractional Baud Rate Register
    volatile uint32_t lcrh;   // Line Control Register
    volatile uint32_t cr;     // Control Register
    volatile uint32_t ifls;   // Interrupt FIFO Level Select Register
    volatile uint32_t imsc;   // Interrupt Mask Set/Clear Register
    volatile uint32_t ris;    // Raw Interrupt Status Register
    volatile uint32_t mis;    // Masked Interrupt Status Register
    volatile uint32_t icr;    // Interrupt Clear Register
    volatile uint32_t dmacr;  // DMA Control Register
} pl011_t;                    // PL011 UART

#define uart0 ((pl011_t *)UART0_BASE) // Base address for PL011 UART0
#define uart1 ((pl011_t *)UART1_BASE) // Base address for PL011 UART1

#define UART0_BASE 0x16000000 // Base address for PL011 UART
#define UART1_BASE 0x17000000 // Base address for PL011 UART
#define PL011_CLOCK 24000000  // Clock frequency for PL011 UART

// Flag Register bits
#define UART_FR_TXFF (1 << 5) // Transmit FIFO full
#define UART_FR_RXFE (1 << 4) // Receive FIFO empty
#define UART_FR_BUSY (1 << 3) // UART busy
#define UART_FR_CTS (1 << 0)  // Clear to Send

// Line Control Register bits
#define UART_LCRH_WLEN_5_gc (0 << 5) // 5 data bits
#define UART_LCRH_WLEN_6_gc (1 << 5) // 6 data bits
#define UART_LCRH_WLEN_7_gc (2 << 5) // 7 data bits
#define UART_LCRH_WLEN_8_gc (3 << 5) // 8 data bits
#define UART_LCRH_FEN (1 << 4)       // Enable FIFOs
#define UART_LCRH_STP2 (1 << 3)      // Two stop bits
#define UART_LCRH_EPS (1 << 2)       // Even parity select
#define UART_LCRH_PEN (1 << 1)       // Parity enable

// Control Register bits
#define UART_CR_CTSEN (1 << 15) // CTS hardware flow control enable
#define UART_CR_RTSEN (1 << 14) // RTS hardware flow control enable
#define UART_CR_OUT2 (1 << 13)  // Out2
#define UART_CR_OUT1 (1 << 12)  // Out1
#define UART_CR_RTS (1 << 11)   // RTS
#define UART_CR_DTR (1 << 10)   // DTR
#define UART_CR_RXE (1 << 9)    // Receive enable
#define UART_CR_TXE (1 << 8)    // Transmit enable
#define UART_CR_LBE (1 << 7)    // Loopback enable
#define UART_CR_UARTEN (1 << 0) // UART enable

// Interrupt FIFO Level Select Register bits
#define UART_IFLS_RXIFLSEL_1_8th_gc (0 << 3) // RX FIFO interrupt level 0
#define UART_IFLS_RXIFLSEL_1_4th_gc (1 << 3) // RX FIFO interrupt level 1/4
#define UART_IFLS_RXIFLSEL_1_2th_gc (2 << 3) // RX FIFO interrupt level 1/2
#define UART_IFLS_RXIFLSEL_3_4th_gc (3 << 3) // RX FIFO interrupt level 3/4
#define UART_IFLS_RXIFLSEL_7_8th_gc (4 << 3) // RX FIFO interrupt level 7/8

// Interrupt Mask Set/Clear Register bits
#define UART_IMSC_OEIM (1 << 10)  // Overrun error interrupt mask
#define UART_IMSC_BEIM (1 << 9)   // Break error interrupt mask
#define UART_IMSC_PEIM (1 << 8)   // Parity error interrupt mask
#define UART_IMSC_FEIM (1 << 7)   // Framing error interrupt mask
#define UART_IMSC_RTIM (1 << 6)   // Receive timeout interrupt mask
#define UART_IMSC_TXIM (1 << 5)   // Transmit interrupt mask
#define UART_IMSC_RXIM (1 << 4)   // Receive interrupt mask
#define UART_IMSC_DSRMIM (1 << 3) // DSR match interrupt mask
#define UART_IMSC_DCDMIM (1 << 2) // DCD match interrupt mask
#define UART_IMSC_CTSMIM (1 << 1) // CTS match interrupt mask
#define UART_IMSC_RIMIM (1 << 0)  // Ring indicator match interrupt mask

// Masked Interrupt Status register bits
#define UART_MIS_OEMIS (1 << 10)  // Overrun error interrupt status
#define UART_MIS_BEMIS (1 << 9)   // Break error interrupt status
#define UART_MIS_PEMIS (1 << 8)   // Parity error interrupt status
#define UART_MIS_FEMIS (1 << 7)   // Framing error interrupt status
#define UART_MIS_RTMIS (1 << 6)   // Receive timeout interrupt status
#define UART_MIS_TXMIS (1 << 5)   // Transmit interrupt status
#define UART_MIS_RXMIS (1 << 4)   // Receive interrupt status
#define UART_MIS_DSRMMIS (1 << 3) // DSR match interrupt status
#define UART_MIS_DCDMMIS (1 << 2) // DCD match interrupt status
#define UART_MIS_CTSMMIS (1 << 1) // CTS match interrupt status
#define UART_MIS_RIMMIS (1 << 0)  // Ring indicator match interrupt status

#endif