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

    /* Tests that should succeed */
    int8_t ret = fat32_create_directory("/test"); // Create directory in root
    ret = fat32_create_directory("/test/test1");  // Create nested directory
    ret = fat32_create_file("/cj.txt");           // Create file
    ret = fat32_create_file("/test/cj.txt");      // Create file in directory

    int8_t fd = fat32_open("/cj.txt"); // Open file
    char buf[] = "This is a test for writing\n";
    fat32_write(fd, buf, sizeof(buf)); // Write to file
    char read_buf[100] = {0};
    fat32_seek(fd, 0, SEEK_SET);
    uint32_t bytes_read = fat32_read(fd, read_buf, 100); // Read file
    printk("Read %u bytes. Contents:\n%s", bytes_read, read_buf);
    fat32_close(fd);

    ret = fat32_delete("/test/test1"); // Delete an empty directory
    printk("Return value (/test/test1 delete): %d\n", ret);
    ret = fat32_delete("/hello.txt"); // Delete file
    printk("Return value (hello.txt delete): %d\n", ret);
    ret = fat32_delete("/cj.txt"); // Delete file created by me
    printk("Return value (cj.txt delete): %d\n", ret);

    /* Tests that should fail */
    ret = fat32_open("/test"); // Open directory as file
    printk("Opening test directory. Return value: %d\n", ret);

    ret = fat32_create_directory("/test"); // Create already made directory
    printk("Creating test directory again (fail). Return value: %d\n", ret);
    ret = fat32_create_file("/log.txt"); // Create already made file
    printk("Creating log.txt file again (fail). Return value: %d\n", ret);
    ret = fat32_delete("/cj.txt");
    printk("Deleting already deleted file (fail). Return value: %d\n", ret);

    ret = fat32_write(5, buf, sizeof(buf)); // Give bad fd
    printk("Writing to non-existent fd. Return value: %d\n", ret);
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