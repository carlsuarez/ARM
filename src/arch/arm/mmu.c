#include "arch/arm/mmu.h"

static uint32_t *const l1_page_table = (uint32_t *)&_l1_page_table_start;
static uint32_t *const coarse_pt0 = (uint32_t *)&_coarse_pt0_start;
void init_page_tables(void)
{
    uint32_t free_pages_start = (uint32_t)&_free_pages_start;
    uint32_t kernel_end = (uint32_t)&_kernel_end;

    // Zero out l1 page table
    memset(l1_page_table, 0, NUM_L1_ENTRIES * sizeof(uint32_t));

    // Create section for first 1MB of memory
    l1_page_table[0] = SECTION_ENTRY(0, AP_USER_NONE, DOMAIN_KERNEL);

    // Set up coarse table for page allocator
    l1_page_table[(uint32_t)FREE_PAGES_VA_BASE >> 20] = COARSE_ENTRY((uint32_t)coarse_pt0, DOMAIN_USER);

    for (uint16_t i = 0; i < NUM_L2_ENTRIES; i++)
    {
        uint32_t pa = free_pages_start + i * 0x1000;
        coarse_pt0[i] = L2_PAGE_ENTRY(pa, AP_USER_NONE, AP_USER_NONE, AP_USER_NONE, AP_USER_NONE);
    }

    // Create sections for hardware
    l1_page_table[IDENTITY_MAP_L1(UART0_BASE)] = SECTION_ENTRY(UART0_BASE, AP_USER_NONE, DOMAIN_HW);
    l1_page_table[IDENTITY_MAP_L1(UART1_BASE)] = SECTION_ENTRY(UART1_BASE, AP_USER_NONE, DOMAIN_HW);

    l1_page_table[IDENTITY_MAP_L1(MMCI_BASE)] = SECTION_ENTRY(MMCI_BASE, AP_USER_NONE, DOMAIN_HW);

    l1_page_table[IDENTITY_MAP_L1(PIC_BASE)] = SECTION_ENTRY(PIC_BASE, AP_USER_NONE, DOMAIN_HW);

    l1_page_table[IDENTITY_MAP_L1(TIMER0_BASE)] = SECTION_ENTRY(TIMER0_BASE, AP_USER_NONE, DOMAIN_HW);
}

int8_t set_page_ap(uintptr_t coarse_pt, uint8_t page_index, uint8_t ap)
{
    uint32_t coarse_entry = l1_page_table[coarse_pt >> 20];
    if (!(coarse_entry & 0b01) || (coarse_entry & ((1 << 9) | (0b11 << 2))) || !(coarse_entry & (1 << 4))) // Not a valid coarse entry descriptor
        return -1;

    // Get address of coarse_table
    uint32_t *coarse_table = (uint32_t *)(coarse_entry & 0xFFFFFB00);

    // Set the ap of the page_entry
    uint32_t page_entry = coarse_table[page_index];
    page_entry &= ~(0xFF << 4);
    page_entry |= ap << 4;
    coarse_table[page_index] = page_entry;
}