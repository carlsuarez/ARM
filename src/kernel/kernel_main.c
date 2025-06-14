#include <stdint.h>
#include "kernel/printk.h"
#include "hw/pic.h"
#include "drivers/timer.h"
#include "kernel/interrupt.h"
#include "kernel/task/task.h"
#include "lib/syscall.h"
#include "lib/printf.h"
#include "lib/libtask.h"
#include "drivers/mmci.h"
#include "fs/fat32/fat32.h"
#include "kernel/kheap.h"
#include "kernel/shell/commands.h"
#include "arch/arm/mmu.h"
#include "kernel/page_alloc.h"

// Function prototypes
void task1(void);
void task2(void);

int kernel_main(void)
{
    sei();
    sef();

    PIC_IRQ_CLEAR(); // Disable all interrupts
    PIC_FIQ_CLEAR(); // Disable all interrupts

    uart0_init(115200); // Initialize UART with 115200 baud rate, 2 stop bits, 8 data bits, no parity
    timer1_init(1e3, TIMER_MODE_PERIODIC, TIMER_IE, TIMER_PRESCALE_NONE_gc, TIMER_SIZE_32, 0);
    printk("Kernel main started\n");

    pic->IRQ_ENABLESET = PIC_TIMERINT1 | PIC_UARTINT0 | PIC_UARTINT1;

    kheap_init((uintptr_t)&_kernel_heap_start);
    init_page_alloc();

    task_init();
    task_create("task 1", (uintptr_t)task1);
    task_create("task 2", (uintptr_t)task2);

    mmci_card_init();
    fat32_init(0);

    uint32_t *l1 = (uint32_t *)&_l1_page_table_start;
    uint32_t *cpt = (uint32_t *)&_coarse_pt0_start;
    printk("l1_page_table: 0x%x\n", (uint32_t)l1);
    printk("first entry: 0x%x\n", *l1);
    printk("coarse_pt0: 0x%x\n", (uint32_t)cpt);

    int8_t fd = fat32_open("/docs/info.txt");
    printk("fd: %d\n", fd);
    char buf[256];
    int32_t ret = fat32_read(fd, buf, sizeof(buf));
    printk("Return val: %d. Contents:\n%s\n", ret, buf);

    uint8_t *page = alloc_page();
    memset(page, 0xAB, PAGE_SIZE);
    printk("Page address: 0x%x\n", (uint32_t)page);
    printk("First value: %x\n", *page);
    uint8_t *page2 = alloc_page();
    memset(page2, 0xDE, PAGE_SIZE);
    printk("Page address: 0x%x\n", (uint32_t)page2);
    printk("First value: %x\n", *page2);

    free_page(page);
    printk("Page after free: %u\n", *page);

    free_page(page2);
    printk("Page after free: %u\n", *page2);

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