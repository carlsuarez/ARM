#ifndef SD_H
#define SD_H

#include <stdint.h>

// Command indices
#define GO_IDLE_STATE 0
#define ALL_SEND_CID 2
#define SEND_RELATIVE_ADDR 3
#define SET_DSR 4
#define SWITCH_FUNCTION 6
#define SELECT_DESELECT_CARD 7
#define SEND_IF_COND 8
#define SEND_CSD 9
#define SEND_CID 10
#define STOP_TRANSMISSION 12
#define SEND_STATUS 13
#define GO_INACTIVE_STATE 15
#define SET_BLOCKLEN 16
#define READ_SINGLE_BLOCK 17
#define READ_MULTIPLE_BLOCK 18
#define WRITE_BLOCK 24
#define WRITE_MULTIPLE_BLOCK 25
#define PROGRAM_CSD 27
#define SET_WRITE_PROT 28
#define CLR_WRITE_PROT 29
#define SEND_WRITE_PROT 30
#define ERASE_WR_BLK_START 32
#define ERASE_WR_BLK_END 33
#define ERASE 38
#define LOCK_UNLOCK 42
#define APP_CMD 55
#define GEN_CMD 56

// Application command indices
#define SET_BUS_WIDTH 6
#define SD_STATUS 13
#define SEND_NUM_WR_BLOCKS 22
#define SET_WR_BLK_ERASE_COUNT 23
#define SD_SEND_OP_COND 41
#define SET_CLR_CARD_DETECT 42
#define SEND_SCR 51

#endif