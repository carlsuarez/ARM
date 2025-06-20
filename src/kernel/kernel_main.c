#include "defs.h"
#include "libc/kernel/printk.h"
#include "hw/pic.h"
#include "drivers/timer.h"
#include "kernel/interrupt.h"
#include "kernel/task/task.h"
#include "drivers/mmci.h"
#include "fs/fat/fat32.h"
#include "libc/kernel/malloc.h"
#include "kernel/shell/commands.h"
#include "arch/arm/mmu.h"
#include "libc/kernel/page_alloc.h"
#include "kernel/task/elf.h"

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
    init_page_alloc();

    task_init();

    mmci_card_init();
    fat32_init(0);

    printk("Kernel main\n");

    fat32_dir_entry_t dir_ent;
    fat32_stat("/test.txt", &dir_ent);

    uintptr_t va = 0x007f0000;
    uintptr_t coarse_pt = vm_alloc(va, dir_ent.file_size);
    printk("coarse_pt 0x%x\n", coarse_pt);
    int8_t fd = fat32_open("/test.txt");
    fat32_read(fd, (void *)va, dir_ent.file_size);
    printk("Contents\n%s", (char *)va);

    // uintptr_t entry = elf_load("/user.elf", 0x007f0000);

    vm_free(va);
    clf();
    cli();

    // TIMER1_START();

    while (1)
        ;
}