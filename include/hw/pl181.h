#ifndef PL181_H
#define PL181_H
#include <stdint.h>
#include "defs.h"

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

#define MMIC_BASE 0x1C000000
#define mmci ((mmci_t *)MMIC_BASE) // Base address for MMCI

#endif