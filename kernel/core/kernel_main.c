#include <defs.h>
#include <kernel/lib/printk.h>
#include <kernel/hw/pic.h>
#include <kernel/drivers/timer.h>
#include <kernel/arch/arm/interrupt.h>
#include <kernel/core/task/task.h>
#include <kernel/drivers/mmci.h>
#include <kernel/fs/fat/fat32.h>
#include <kernel/lib/malloc.h>
#include <kernel/core/shell/commands.h>
#include <kernel/arch/arm/mmu.h>
#include <kernel/lib/page_alloc.h>
#include <kernel/core/task/elf/elf.h>

int kernel_main(void)
{
    sei();
    sef();

    PIC_IRQ_CLEAR(); // Disable all interrupts
    PIC_FIQ_CLEAR(); // Disable all interrupts

    uart0_init(115200); // Initialize UART with 115200 baud rate, 2 stop bits, 8 data bits, no parity
    timer1_init(1e6, TIMER_MODE_PERIODIC, TIMER_IE, TIMER_PRESCALE_NONE_gc, TIMER_SIZE_32, 0);

    pic->IRQ_ENABLESET = PIC_TIMERINT1 | PIC_UARTINT0 | PIC_UARTINT1;

    kheap_init((uintptr_t)&_kernel_heap_start);
    init_page_allocator(ALLOC_1K, COARSE_TABLE_ALLOCATOR_SPACE_SIZE / TINY_PAGE_SIZE, TINY_PAGE_SIZE, (uintptr_t)&_coarsepagetables_space_start);
    init_page_allocator(ALLOC_4K, PAGE_ALLOCATOR_SPACE_SIZE / SMALL_PAGE_SIZE, SMALL_PAGE_SIZE, (uintptr_t)&_free_pages_start);
    init_page_allocator(ALLOC_16K, L1_TABLE_ALLOCATOR_SPACE_SIZE / L1_TABLE_SIZE, L1_TABLE_SIZE, (uintptr_t)&_l1pagetables_space_start);

    task_init();

    mmci_card_init();
    fat32_init(0);

    printk("Kernel main\n");

    task_create("/main.elf", "main");
    clf();
    cli();

    printk("Starting timer\n");
    TIMER1_START();

    while (1)
        ;
}