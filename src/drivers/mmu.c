#include "kernel/mmu.h"

void init_page_tables(void)
{
    uint32_t *l1_page_table = (uint32_t *)&_l1_page_table_start;
    uint32_t *coarse_pt0 = (uint32_t *)&_coarse_pt0_start;

    // Create secction for first 1MB of memory
    l1_page_table[0] = SECTION_ENTRY(0, AP_USER_RW, DOMAIN_MANAGER);

    // Create section for hardware
    l1_page_table[UART0_BASE >> 20] = SECTION_ENTRY(UART0_BASE, AP_USER_RW, DOMAIN_MANAGER);
    l1_page_table[UART1_BASE >> 20] = SECTION_ENTRY(UART1_BASE, AP_USER_RW, DOMAIN_MANAGER);

    l1_page_table[MMCI_BASE >> 20] = SECTION_ENTRY(MMCI_BASE, AP_USER_RW, DOMAIN_MANAGER);

    l1_page_table[PIC_BASE >> 20] = SECTION_ENTRY(PIC_BASE, AP_USER_RW, DOMAIN_MANAGER);

    l1_page_table[TIMER0_BASE >> 20] = SECTION_ENTRY(TIMER0_BASE, AP_USER_RW, DOMAIN_MANAGER);
}