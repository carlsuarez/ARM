#ifndef MMU_H
#define MMU_H

#include <defs.h>
#include <kernel/hw/pl011.h>
#include <kernel/hw/pl181.h>
#include <kernel/hw/pic.h>
#include <kernel/hw/timer.h>
#include <common/memory.h>
#include <kernel/lib/page_alloc.h>

#define NUM_L1_ENTRIES 4096
#define NUM_COARSE_ENTRIES 256
#define SMALL_PAGE_SIZE 4096
#define TINY_PAGE_SIZE 1024
#define L1_TABLE_SIZE 16384

#define L2_TYPE_INVALID 0
#define L2_TYPE_LARGE 1
#define L2_TYPE_SMALL 2
#define L2_TYPE_TINY 3

#define AP_USER_NONE 1
#define AP_USER_READ 2
#define AP_USER_RW 3

#define AP0(ap) (ap)
#define AP1(ap) ((ap) << 2)
#define AP2(ap) ((ap) << 4)
#define AP3(ap) ((ap) << 6)
#define AP(ap) (AP0(ap) | AP1(ap) | AP2(ap) | AP3(ap))

#define C_WB 0
#define C_WT 1
#define B_BUF 0
#define B_NBUF 1

#define COARSE_TABLE_ALLOCATOR_SPACE_SIZE ((uint32_t)&_coarsepagetables_space_end - (uint32_t)&_coarsepagetables_space_start)
#define PAGE_ALLOCATOR_SPACE_SIZE ((uint32_t)&_free_pages_end - (uint32_t)&_free_pages_start)
#define L1_TABLE_ALLOCATOR_SPACE_SIZE ((uint32_t)&_l1pagetables_space_end - (uint32_t)&_l1pagetables_space_start)
#define SECTION_SIZE 0x100000
#define SECTION_MASK 0xFFF00000
#define COARSE_MASK 0xFFFFFC00
#define PAGE_MASK 0xFFFFF000
#define PAGE_OFFSET_MASK 0xFFF

#define SECTION_ENTRY(phys_addr, ap, domain)                              \
    (((phys_addr) & 0xFFF00000) | /* bits 31:20 = section base address */ \
     ((ap) << 10) |               /* bits 11:10 = access permissions */   \
     ((domain) << 5) |            /* bits 8:5   = domain number */        \
     (1 << 4) |                   /* bit 4 = always 1 for sections */     \
     (2))                         /* bits 1:0 = 0b10 for section descriptor */
#define COARSE_ENTRY(phys_addr, domain)                                          \
    (((phys_addr) & 0xFFFFFC00) | /* bits 31:10 = coarse page table base addr */ \
     ((domain) << 5) |            /* bits 8:5 = domain */                        \
     (1 << 4) |                   /* bit 4 = always 1 */                         \
     (1))                         /* bits 1:0 = 0b01 for coarse descriptor */
#define L2_PAGE_ENTRY(phys_addr, ap, c, b)                              \
    (((phys_addr) & 0xFFFFF000) | /* bits 31:12 = 4KB page base addr */ \
     ((ap) << 4) |                /* bits 11:4 = AP bits */             \
     ((c) << 3) |                 /* bit 3 = C bit */                   \
     ((b) << 2) |                 /* bit 2 = B bit */                   \
     L2_TYPE_SMALL)               /* bits 1:0 = 0b10 for small page */

#define L1_INDEX(virt_addr) ((virt_addr) >> 20)                      // Index into the l1 page table for a virtual address
#define L2_INDEX(virt_addr) (((virt_addr) >> 12) & 0xFF)             // Index into an l2 page table for a virtual address
#define COARSE_BASE(l1_entry) ((l1_entry) & 0xFFFFFC00)              // Get base address of the coarse page table from the l1 entry
#define COARSE_PAGE_BASE(coarse_entry) ((coarse_entry) & 0xFFFFF000) // Get base address of the physical page from the coarse table entry

#define DOMAIN_KERNEL 0
#define DOMAIN_USER 1
#define DOMAIN_HW 2
#define DOMAIN_MIXED 3
#define DOMAIN_NO_ACCESS 15

extern volatile uint32_t l1_page_table[NUM_L1_ENTRIES];

static inline void set_s_bit(void)
{
    uint32_t sctlr;
    asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(sctlr));
    sctlr |= (1 << 9); // Set S bit (bit 9)
    asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(sctlr));
}

static inline void clear_s_bit(void)
{
    uint32_t sctlr;
    asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(sctlr));
    sctlr &= ~(1 << 9); // Clear S bit (bit 9)
    asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(sctlr));
}

static inline void set_r_bit(void)
{
    uint32_t sctlr;
    asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(sctlr));
    sctlr |= (1 << 8); // Set R bit (bit 8)
    asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(sctlr));
}

static inline void clear_r_bit(void)
{
    uint32_t sctlr;
    asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(sctlr));
    sctlr &= ~(1 << 8); // Clear R bit (bit 8)
    asm volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(sctlr));
}

static inline bool is_valid_l1_section_entry(uint32_t entry)
{
    /* Section descriptor: bits [1:0] == 0b10, bit 4 = 1 */
    return ((entry & 0x3) == 0x2) && (entry & (1 << 4));
}

static inline bool is_valid_l1_coarse_entry(uint32_t entry)
{
    /* Coarse descriptor: bits [1:0] == 0b01, bit 4 = 1 */
    return ((entry & 0x3) == 0x1) && (entry & (1 << 4));
}

static inline bool is_valid_l2_coarse_entry(uint32_t entry)
{
    return (entry & 0x3) == L2_TYPE_SMALL;
}

void init_page_table(uint32_t *l1);

/**
 * @brief Sets the access permissions (AP) for a specific page in a coarse page table.
 *
 * This function updates the access permissions (AP) bits for a page entry within a coarse page table,
 * given the base address of the coarse page table, the page index, and the desired AP value.
 *
 * @param coarse_pt   The virtual address whose corresponding L1 entry points to the coarse page table.
 * @param page_index  The index of the page entry within the coarse page table (0-255).
 * @param ap          The access permission value to set (should be properly encoded for the MMU).
 *
 * @return 0 on success, -1 if the L1 entry is not a valid coarse page table descriptor.
 */
int8_t set_page_ap(uintptr_t coarse_pt, uint8_t page_index, uint8_t ap);

/**
 * @brief Maps a user page in the ARM MMU using a coarse page table.
 *
 * This function sets up a mapping for a single user page by updating the
 * specified coarse page table entry with the provided physical address and access permissions.
 *
 * @param coarse_pt_phys Physical address of the coarse page table.
 * @param va Virtual address to be mapped.
 * @param page_phys Physical address of the page to map.
 * @param ap Access permissions.
 */
void map_page(uintptr_t coarse_pt_phys, uintptr_t va, uintptr_t page_phys, uint8_t ap);

void set_l1_entry(uintptr_t va, uint32_t entry);

static void *translate_addr(uint32_t *l1, uintptr_t va)
{
    uint32_t l1_entry = l1[L1_INDEX(va)];

    if (is_valid_l1_section_entry(l1_entry))
    {
        return (void *)((l1_entry & SECTION_MASK) + (va & ~SECTION_MASK));
    }

    if (is_valid_l1_coarse_entry(l1_entry))
    {
        uint32_t *coarse_pt = (uint32_t *)(COARSE_BASE(l1_entry));
        uint32_t coarse_entry = coarse_pt[L2_INDEX(va)];
        return (void *)(COARSE_PAGE_BASE(coarse_entry) + (va & PAGE_OFFSET_MASK));
    }

    return NULL;
}

#endif
