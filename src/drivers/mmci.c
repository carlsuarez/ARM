#include "drivers/mmci.h"
#include "hw/sd.h"
#include "kernel/printk.h"

static uint8_t is_sd_v2;

static inline void delay_ms(int32_t ms)
{
    for (volatile int32_t i = 0; i < ms * 10000; ++i)
        __asm__ volatile("nop");
}

static int32_t mmci_send_command(uint8_t index, uint32_t arg, uint32_t flags, uint32_t *response_out)
{
    // Clear status flags before sending a new command
    mmci->clear = 0x7FF;

    // Set command argument
    mmci->argument = arg;

    // Build command
    uint32_t command = MMCI_COMMAND_INDEX(index) | MMCI_COMMAND_ENABLE;
    if (flags & MMCI_COMMAND_RESPONSE)
        command |= MMCI_COMMAND_RESPONSE;
    if (flags & MMCI_COMMAND_LONG_RSP)
        command |= MMCI_COMMAND_LONG_RSP;

    // Send command
    mmci->command = command;

    // Wait for response or error
    while (!(mmci->status & (MMCI_STATUS_CMDSENT | MMCI_STATUS_CMDRESPEND |
                             MMCI_STATUS_CMDTIMEOUT | MMCI_STATUS_CMDCRCFAIL)))
        ;

    // Handle errors
    if (mmci->status & MMCI_STATUS_CMDTIMEOUT)
    {
        mmci->clear = MMCI_CLEAR_CMDTIMEOUTCLR;
        return -1;
    }
    if (mmci->status & MMCI_STATUS_CMDCRCFAIL)
    {
        mmci->clear = MMCI_CLEAR_CMDCRCFAILCLR;
        return -2;
    }

    // Read response if needed
    if (flags & MMCI_COMMAND_RESPONSE)
    {
        if (flags & MMCI_COMMAND_LONG_RSP)
        {
            if (response_out)
            {
                response_out[0] = mmci->response[0];
                response_out[1] = mmci->response[1];
                response_out[2] = mmci->response[2];
                response_out[3] = mmci->response[3];
            }
        }
        else
        {
            if (response_out)
                response_out[0] = mmci->response[0];
        }
    }

    // Clear status flags after handling
    mmci->clear = MMCI_CLEAR_CMDRESPENDCLR | MMCI_CLEAR_CMDSENTCLRCLR;

    return 0;
}

int32_t mmci_card_init(void)
{
    uint32_t response[4];

    uint32_t arg, command_ret;
    uint32_t retry = 0;

    // Step 1: Power up the card
    mmci->power = MMCI_POWER_UP | MMCI_POWER_OP_V(3.3); // Power-up at 3.3V
    delay_ms(10);
    mmci->power = MMCI_POWER_ON | MMCI_POWER_OP_V(3.3); // Power-on phase
    delay_ms(10);

    // Step 2: Enable clock (lowest speed, clock divider 0xFF)
    mmci->clock = MMCI_CLK_CLKDIV(0xFF) | MMCI_CLK_EN;

    // Step 3: CMD0 - GO_IDLE_STATE
    arg = 0;
    command_ret = mmci_send_command(GO_IDLE_STATE, arg, 0, NULL);
    if (command_ret)
        printk("CM0 failed. Return value: %d\n", command_ret);

    delay_ms(2);

    // Step 4: CMD8 - SEND_IF_COND (to check SD v2.0)
    /*
    Argument:
    [11:8]supply voltage(VHS)
    [7:0]check pattern
    */

    arg = (MMCI_POWER_OP_V(3.3) << 8) | 0b10101010;
    command_ret = mmci_send_command(SEND_IF_COND, arg, MMCI_COMMAND_RESPONSE, response);

    if (command_ret == 0)
    {
        if (response[0] == arg)
        {
            is_sd_v2 = 1;
            printk("SD v2.x card detected\n");
        }
    }
    else
    {
        is_sd_v2 = 0;
        printk("SD v1.x card detected\n");
    }

    // Step 5: ACMD41 - Initialize card (repeat until busy bit set)
    arg = is_sd_v2 ? 0x40300000 : 0x00300000;
    do
    {
        command_ret = mmci_send_command(APP_CMD, 0, MMCI_COMMAND_RESPONSE, NULL);
        if (command_ret)
        {
            printk("APP_CMD failed. Return value: %d\n", command_ret);
            return -2;
        }
        command_ret = mmci_send_command(SD_SEND_OP_COND, arg, MMCI_COMMAND_RESPONSE, response);
        if (command_ret)
        {
            printk("ACMD41 failed. Return value: %d\n", command_ret);
            return -3;
        }
        retry++;
        delay_ms(10);
    } while (!(response[0] & (1U << 31)) && retry < 100);

    if (!(response[0] & (1U << 31)))
    {
        uart_puts(uart0, "Card did not complete initialization\n");
        return -4;
    }

    // Step 6: CMD2 - ALL_SEND_CID
    command_ret = mmci_send_command(ALL_SEND_CID, 0, MMCI_COMMAND_RESPONSE | MMCI_COMMAND_LONG_RSP, response);
    if (command_ret)
    {
        printk("CMD2 failed. Return value: %d\n", command_ret);
        return -5;
    }

    // Step 7: CMD3 - SEND_RELATIVE_ADDR (RCA)
    command_ret = mmci_send_command(SEND_RELATIVE_ADDR, 0, MMCI_COMMAND_RESPONSE, response);
    if (command_ret)
    {
        printk("CMD3 failed. Return value: %d\n", command_ret);
        return -6;
    }

    uint32_t rca = response[0] & 0xFFFF0000;

    // Step 8: CMD7 - SELECT_CARD
    command_ret = mmci_send_command(SELECT_DESELECT_CARD, rca, MMCI_COMMAND_RESPONSE, NULL);
    if (command_ret)
    {
        printk("CMD7 failed. Return value: %d\n", command_ret);
        return -7;
    }

    printk("SD card initialized successfully!\n");
    return 0;
}

int8_t sd_read_block(uint32_t block_addr, uint8_t *buf)
{
    // Adjust address for SD v1 (byte addressing)
    if (!is_sd_v2)
        block_addr *= 512;

    mmci->clear = 0x7FF;

    mmci->argument = block_addr;
    mmci->data_timer = 0xFFFFFFFF;
    mmci->data_length = 512;

    mmci->data_control = MMCI_DATA_CONTROL_EN |
                         MMCI_DATA_CONTROL_DIR |
                         MMCI_DATA_CONTROL_BLOCK_SIZE(9); // 2^9 = 512

    mmci->command = MMCI_COMMAND_INDEX(READ_SINGLE_BLOCK) |
                    MMCI_COMMAND_RESPONSE |
                    MMCI_COMMAND_ENABLE;

    // Wait for data transfer to begin
    int i = 0;
    while (!(mmci->status & MMCI_STATUS_RXACTIVE))
    {
        if (++i > 1000000)
            return -1;
    }

    // Read data from FIFO
    for (int j = 0; j < 512 / 4; j++)
    {
        while (!(mmci->status & MMCI_STATUS_RXDATAAVLBL))
        {
            if (++i > 1000000)
                return -2;
        }
        ((uint32_t *)buf)[j] = mmci->fifo[0];
    }

    // Wait for data transfer complete
    while (!(mmci->status & MMCI_STATUS_DATAEND))
    {
        if (++i > 1000000)
            return -3;
    }

    return 0;
}

int8_t sd_write_block(uint32_t block_addr, const uint8_t *buf)
{
    // Adjust address for SD v1 (byte addressing)
    if (!is_sd_v2)
        block_addr *= 512;

    mmci->clear = 0x7FF;

    mmci->argument = block_addr;
    mmci->data_timer = 0xFFFFFFFF;
    mmci->data_length = 512;

    mmci->data_control = MMCI_DATA_CONTROL_EN |
                         MMCI_DATA_CONTROL_BLOCK_SIZE(9); // 2 ** 9 = 512 bytes

    mmci->command = MMCI_COMMAND_INDEX(WRITE_BLOCK) |
                    MMCI_COMMAND_RESPONSE |
                    MMCI_COMMAND_ENABLE;

    int i = 0;
    while (!(mmci->status & MMCI_STATUS_TXACTIVE))
    {
        if (++i > 1000000)
            return -1;
    }

    for (int j = 0; j < 512 / 4; j++)
    {
        while (!(mmci->status & MMCI_STATUS_TXFIFOEMPTY))
        {
            if (++i > 1000000)
                return -2;
        }
        mmci->fifo[0] = ((const uint32_t *)buf)[j];
    }

    while (!(mmci->status & MMCI_STATUS_DATAEND))
    {
        if (++i > 1000000)
            return -3;
    }

    return 0;
}
