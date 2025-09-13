#include <kernel/arch/arm/mmu.h>

void init_page_table(uint32_t *l1)
{
    uint32_t free_pages_start = (uint32_t)&_free_pages_start;
    uint32_t coarse_page_tables_start = (uint32_t)&_coarsepagetables_space_start;
    uint32_t l1_page_tables_start = (uint32_t)&_l1pagetables_space_start;

    // Zero out l1 page table
    memset((void *)l1, 0, NUM_L1_ENTRIES * sizeof(uint32_t));

    // Create section for first 1MB of memory
    l1[0] = SECTION_ENTRY(0, AP_USER_NONE, DOMAIN_KERNEL);

    // Create sections for coarse page table allocator
    for (uint8_t i = 0; i < COARSE_TABLE_ALLOCATOR_SPACE_SIZE / SECTION_SIZE; i++)
    {
        uintptr_t addr = coarse_page_tables_start + i * SECTION_SIZE;
        l1[L1_INDEX(addr)] = SECTION_ENTRY(addr, AP_USER_NONE, DOMAIN_KERNEL);
    }

    // Create sections for small page allocator
    for (uint8_t i = 0; i < PAGE_ALLOCATOR_SPACE_SIZE / SECTION_SIZE; i++)
    {
        uintptr_t addr = free_pages_start + i * SECTION_SIZE;
        l1[L1_INDEX(addr)] = SECTION_ENTRY(addr, AP_USER_NONE, DOMAIN_KERNEL);
    }

    // Create sections for coarse page table allocator
    for (uint8_t i = 0; i < COARSE_TABLE_ALLOCATOR_SPACE_SIZE / SECTION_SIZE; i++)
    {
        uintptr_t addr = coarse_page_tables_start + i * SECTION_SIZE;
        l1[L1_INDEX(addr)] = SECTION_ENTRY(addr, AP_USER_NONE, DOMAIN_KERNEL);
    }

    // Create sections for l1 page table allocator
    for (uint8_t i = 0; i < L1_TABLE_ALLOCATOR_SPACE_SIZE / SECTION_SIZE; i++)
    {
        uintptr_t addr = l1_page_tables_start + i * SECTION_SIZE;
        l1[L1_INDEX(addr)] = SECTION_ENTRY(addr, AP_USER_NONE, DOMAIN_KERNEL);
    }

    // Create sections for hardware
    l1[L1_INDEX(UART0_BASE)] = SECTION_ENTRY(UART0_BASE, AP_USER_NONE, DOMAIN_HW);
    l1[L1_INDEX(UART1_BASE)] = SECTION_ENTRY(UART1_BASE, AP_USER_NONE, DOMAIN_HW);

    l1[L1_INDEX(MMCI_BASE)] = SECTION_ENTRY(MMCI_BASE, AP_USER_NONE, DOMAIN_HW);

    l1[L1_INDEX(PIC_BASE)] = SECTION_ENTRY(PIC_BASE, AP_USER_NONE, DOMAIN_HW);

    l1[L1_INDEX(TIMER0_BASE)] = SECTION_ENTRY(TIMER0_BASE, AP_USER_NONE, DOMAIN_HW);
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

    return 0;
}

void map_page(uintptr_t coarse_pt_phys, uintptr_t va, uintptr_t page_phys, uint8_t ap)
{
    uint32_t *coarse_ptr = (uint32_t *)coarse_pt_phys;
    uint8_t index = (va >> 12) & 0xFF;
    coarse_ptr[index] = L2_PAGE_ENTRY(page_phys, ap, C_WB, B_BUF);
}

void set_l1_entry(uintptr_t va, uint32_t entry)
{
    l1_page_table[L1_INDEX(va)] = entry;
}
