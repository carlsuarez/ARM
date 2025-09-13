#ifndef PL181_H
#define PL181_H
#include <stdint.h>
#include <defs.h>

typedef struct
{
    volatile uint32_t power;
    volatile uint32_t clock;
    volatile uint32_t argument;
    volatile uint32_t command;
    volatile uint32_t resp_cmd;
    volatile uint32_t response[4];
    volatile uint32_t data_timer;
    volatile uint32_t data_length;
    volatile uint32_t data_control;
    volatile uint32_t data_count;
    volatile uint32_t status;
    volatile uint32_t clear;
    volatile uint32_t mask[2];
    RESERVE_U32(1);
    volatile uint32_t fifo_count;
    RESERVE_U32(13);
    volatile uint32_t fifo[16];
    RESERVE_U32(0x3C4);
    volatile uint32_t periph_id[4];
    volatile uint32_t pcell_id[4];
} mmci_t;

#define MMCI_BASE 0x1C000000
#define mmci ((mmci_t *)MMCI_BASE) // Base address for MMCI

// Power register values
#define MMCI_POWER_OFF (0 << 0)
#define MMCI_POWER_ON (3 << 0)
#define MMCI_POWER_UP (2 << 0)
#define MMCI_POWER_OP_V(v) ((uint8_t)(((v) < 2.0) ? 0 : ((v) > 3.6) ? 15 \
                                                                    : (((v) - 2.0) * (15.0 / 1.6) + 0.5)))
#define MMCI_POWER_OPENDRAIN (1 << 6)
#define MMCI_POWER_ROD (1 << 7)

// Clock register values
#define MMCI_CLK_CLKDIV(n) ((uint8_t)n)
#define MMCI_CLK_EN (1 << 8)
#define MMCI_CLK_PWR_SAVE (1 << 9)
#define MMCI_CLK_BYPASS (1 << 10)

// Command register
#define MMCI_COMMAND_INDEX(i) ((uint8_t)i)
#define MMCI_COMMAND_RESPONSE (1 << 6)  // If set, CPSM waits for a response
#define MMCI_COMMAND_LONG_RSP (1 << 7)  // If set, CPSM recieves a 136-bit long response
#define MMCI_COMMAND_INTERRUPT (1 << 8) // If set, CPSM disables command timer and waits for interrupt request
#define MMCI_COMMAND_PENDING (1 << 9)   // If set, CPSM wait for CmdPend before it starts sending a command
#define MMCI_COMMAND_ENABLE (1 << 10)   // If set, CPSM is enabled

// Data control register
#define MMCI_DATA_CONTROL_EN (1 << 0)
#define MMCI_DATA_CONTROL_DIR (1 << 1)  // 0 = from controller to card, 1 = from card to controller
#define MMCI_DATA_CONTROL_MODE (1 << 2) // 0 = block data transfer, 1 = stream data transfer
#define MMCI_DATA_CONTROL_DMA_EN (1 << 3)
#define MMCI_DATA_CONTROL_BLOCK_SIZE(n) ((uint8_t)n << 4)

// Status register
#define MMCI_STATUS_CMDCRCFAIL (1 << 0)
#define MMCI_STATUS_DATACRCFAIL (1 << 1)
#define MMCI_STATUS_CMDTIMEOUT (1 << 2)
#define MMCI_STATUS_DATATIMEOUT (1 << 3)
#define MMCI_STATUS_TXUNDERRUN (1 << 4)
#define MMCI_STATUS_RXOVERRUN (1 << 5)
#define MMCI_STATUS_CMDRESPEND (1 << 6)
#define MMCI_STATUS_CMDSENT (1 << 7)
#define MMCI_STATUS_DATAEND (1 << 8)
#define MMCI_STATUS_DATABLOCKEND (1 << 10)
#define MMCI_STATUS_CMDACTIVE (1 << 11)
#define MMCI_STATUS_TXACTIVE (1 << 12)
#define MMCI_STATUS_RXACTIVE (1 << 13)
#define MMCI_STATUS_TXFIFOHALFEMPTY (1 << 14)
#define MMCI_STATUS_TXFIFOHALFFULL (1 << 15)
#define MMCI_STATUS_TXFIFOFULL (1 << 16)
#define MMCI_STATUS_RXFIFOFULL (1 << 17)
#define MMCI_STATUS_TXFIFOEMPTY (1 << 18)
#define MMCI_STATUS_RXFIFOEMPTY (1 << 19)
#define MMCI_STATUS_TXDATAAVLBL (1 << 20)
#define MMCI_STATUS_RXDATAAVLBL (1 << 21)

// Clear register
#define MMCI_CLEAR_CMDCRCFAILCLR (1 << 0)
#define MMCI_CLEAR_DATACRCFAILCLR (1 << 1)
#define MMCI_CLEAR_CMDTIMEOUTCLR (1 << 2)
#define MMCI_CLEAR_DATATIMEOUTCLR (1 << 3)
#define MMCI_CLEAR_TXUNDERRUNCLR (1 << 3)
#define MMCI_CLEAR_RXOVERRUNCLR (1 << 5)
#define MMCI_CLEAR_CMDRESPENDCLR (1 << 6)
#define MMCI_CLEAR_CMDSENTCLRCLR (1 << 7)
#define MMCI_CLEAR_DATAENDCLRCLR (1 << 8)
#define MMCI_CLEAR_DATABLOCKENDCLR (1 << 10)

// Interrupt mask register
#define MMCI_IMASK_CMDCRCFAIL (1 << 0)
#define MMCI_IMASK_DATACRCFAIL (1 << 1)
#define MMCI_IMASK_CMDTIMEOUT (1 << 2)
#define MMCI_IMASK_DATATIMEOUT (1 << 3)
#define MMCI_IMASK_TXUNDERRUN (1 << 4)
#define MMCI_IMASK_RXOVERRUN (1 << 5)
#define MMCI_IMASK_CMDRESPEND (1 << 6)
#define MMCI_IMASK_CMDSENT (1 << 7)
#define MMCI_IMASK_DATAEND (1 << 8)
#define MMCI_IMASK_DATABLOCKEND (1 << 10)
#define MMCI_IMASK_CMDACTIVE (1 << 11)
#define MMCI_IMASK_TXACTIVE (1 << 12)
#define MMCI_IMASK_RXACTIVE (1 << 13)
#define MMCI_IMASK_TXFIFOHALFEMPTY (1 << 14)
#define MMCI_IMASK_TXFIFOHALFFULL (1 << 15)
#define MMCI_IMASK_TXFIFOFULL (1 << 16)
#define MMCI_IMASK_RXFIFOFULL (1 << 17)
#define MMCI_IMASK_TXFIFOEMPTY (1 << 18)
#define MMCI_IMASK_RXFIFOEMPTY (1 << 19)
#define MMCI_IMASK_TXDATAAVLBL (1 << 20)
#define MMCI_IMASK_RXDATAAVLBL (1 << 21)

#endif