#include <stdint.h>
#include "kernel/printk.h"
#include "hw/pic.h"
#include "drivers/timer.h"
#include "kernel/interrupt.h"
#include "kernel/task.h"
#include "lib/syscall.h"
#include "lib/printf.h"
#include "lib/task.h"
#include "drivers/mmci.h"
#include "drivers/fat.h"

// Function prototypes
void task1(void);
void task2(void);
void task3(void);

int kernel_main(void)
{
    sei();
    sef();

    PIC_IRQ_CLEAR(); // Disable all interrupts
    PIC_FIQ_CLEAR(); // Disable all interrupts

    uart0_init(115200); // Initialize UART with 115200 baud rate, 2 stop bits, 8 data bits, no parity
    timer1_init(1e6, TIMER_MODE_PERIODIC, TIMER_IE, TIMER_PRESCALE_NONE_gc, TIMER_SIZE_32, 0);
    printk("Kernel main started\n");

    pic->IRQ_ENABLESET = PIC_TIMERINT1 | PIC_UARTINT0 | PIC_UARTINT1;

    task_init();
    task_create(task1);
    task_create(task2);
    task_create(task3);

    mmci_card_init();
    fat32_init(0);

    int8_t fd = fat32_open("/big/bigfile.txt");
    char buf[2500];
    fat32_seek(fd, 0, SEEK_SET);
    int32_t bytes_read = fat32_read(fd, buf, sizeof(buf));
    printk("Read %d bytes\n", bytes_read);
    fat32_truncate(fd, 511);
    memset(buf, 0, sizeof(buf));
    fat32_seek(fd, 0, SEEK_SET);
    bytes_read = fat32_read(fd, buf, sizeof(buf));
    printk("Read %d bytes\n", bytes_read);
    fat32_close(fd);

    fat32_dir_entry_t stat;
    fat32_stat("/big/bigfile.txt", &stat);
    printk("File size: %u\n", stat.file_size);

    clf();
    cli();

    // TIMER1_START();

    while (1)
        ;
}

void task1(void)
{
    const char *str = "Task 1 is running\n";
    printf("%s", str);
    exit();
}

void task2(void)
{
    const uint8_t x = 2, y = 3;
    printf("Task 2 is running\n");
    printf("x + y = %d\n", x + y);
    exit();
}

void task3(void)
{
    const double pi = 3.14159265358979323846;
    printf("Task 3 is running\n");
    printf("Value of pi: %f\n", pi);
    for (volatile uint32_t i = 0; i < 1e9; i++)
        ;
    exit();
}