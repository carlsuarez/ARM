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

    int8_t fd = open("/big/bigfile.txt");
    fat32_file_t *file = get_file_by_fd(fd);
    printk("%x\n", file->current_cluster);

    char buf[3000] = {0};
    int32_t num_read = read(file, buf, sizeof(buf));
    printk("Read %d bytes. Contents:\n%s\n", num_read, buf);
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