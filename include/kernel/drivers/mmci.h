#ifndef MMCI_H
#define MMCI_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/hw/pl181.h>

#define SECTOR_SIZE 512

int32_t mmci_card_init();
int8_t sd_read_block(uint32_t block_addr, uint8_t *buf);
int8_t sd_write_block(uint32_t block_addr, const uint8_t *buf);

#endif