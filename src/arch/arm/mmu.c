#include "arch/arm/mmu.h"

void init_page_tables(void)
{
    uint32_t free_pages_start = (uint32_t)&_free_pages_start;
    uint32_t kernel_end = (uint32_t)&_kernel_end;

    // Zero out l1 page table
    memset((void *)l1_page_table, 0, NUM_L1_ENTRIES * sizeof(uint32_t));

    // Create section for first 1MB of memory
    l1_page_table[0] = SECTION_ENTRY(0, AP_USER_NONE, DOMAIN_KERNEL);

    // Set up coarse table for page allocator
    l1_page_table[MAP_L1((uint32_t)&_free_pages_start)] = COARSE_ENTRY((uint32_t)coarse_pt0, DOMAIN_USER);

    for (uint16_t i = 0; i < NUM_L2_ENTRIES; i++)
    {
        uint32_t pa = free_pages_start + i * 0x1000;
        coarse_pt0[i] = L2_PAGE_ENTRY(pa, AP_USER_NONE, AP_USER_NONE, AP_USER_NONE, AP_USER_NONE);
    }

    // Create sections for hardware
    l1_page_table[MAP_L1(UART0_BASE)] = SECTION_ENTRY(UART0_BASE, AP_USER_NONE, DOMAIN_HW);
    l1_page_table[MAP_L1(UART1_BASE)] = SECTION_ENTRY(UART1_BASE, AP_USER_NONE, DOMAIN_HW);

    l1_page_table[MAP_L1(MMCI_BASE)] = SECTION_ENTRY(MMCI_BASE, AP_USER_NONE, DOMAIN_HW);

    l1_page_table[MAP_L1(PIC_BASE)] = SECTION_ENTRY(PIC_BASE, AP_USER_NONE, DOMAIN_HW);

    l1_page_table[MAP_L1(TIMER0_BASE)] = SECTION_ENTRY(TIMER0_BASE, AP_USER_NONE, DOMAIN_HW);
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

void map_page_user(uintptr_t coarse_pt_phys, uintptr_t va, uintptr_t page_phys, uint8_t ap)
{
    uint32_t *coarse_ptr = (uint32_t *)coarse_pt_phys;
    uint8_t index = (va >> 12) & 0xFF;
    coarse_ptr[index] = L2_PAGE_ENTRY(page_phys, ap << 6, ap << 4, ap << 2, ap);
}

void set_l1_entry(uintptr_t va, uint32_t entry)
{
    l1_page_table[MAP_L1(va)] = entry;
}

uintptr_t vm_alloc(uintptr_t base_va, size_t num_bytes)
{
    uintptr_t coarse_pt = (uintptr_t)alloc_page();

    set_l1_entry(base_va, COARSE_ENTRY(coarse_pt, DOMAIN_USER));

    size_t num_pages = (num_bytes + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < num_pages; i++)
    {
        uintptr_t page = (uintptr_t)alloc_page();
        printk("vm alloc: allocing page @ %p\n", page);
        map_page_user(coarse_pt, base_va, page, 0xFF);
    }

    return coarse_pt;
}

void vm_free(uintptr_t va)
{
    uint32_t *coarse_pt = (uint32_t *)COARSE_BASE(l1_page_table[MAP_L1(va)]);
    printk("vm free: coarse_pt %p\n", coarse_pt);
    for (size_t i = 0; i < NUM_L2_ENTRIES; i++)
    {
        // Free the page
        uintptr_t page = COARSE_PAGE_BASE(coarse_pt[i]);

        // Ignore non-allocated
        if (!page)
            continue;

        printk("vm free: freeing page @ %p\n", page);
        free_page(page);

        // Clear the page
        coarse_pt[i] = 0;
    }

    // Clear the L1 entry
    l1_page_table[MAP_L1(va)] = 0;
}