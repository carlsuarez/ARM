#ifndef MMU_H
#define MMU_H

#include "defs.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>
#include "hw/pl011.h"
#include "hw/pl181.h"
#include "hw/pic.h"
#include "hw/timer.h"
#include "libc/common/memory.h"
#include "libc/kernel/page_alloc.h"

#define NUM_L1_ENTRIES 4096
#define NUM_L2_ENTRIES 256

#define L2_TYPE_INVALID 0
#define L2_TYPE_LARGE 1
#define L2_TYPE_SMALL 2
#define L2_TYPE_TINY 3

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
#define L2_PAGE_ENTRY(phys_addr, ap3, ap2, ap1, ap0)                    \
    (((phys_addr) & 0xFFFFF000) | /* bits 31:12 = 4KB page base addr */ \
     (((ap3) & 0x3) << 10) |      /* bits 11:10 = AP3 bits */           \
     (((ap2) & 0x3) << 8) |       /* bits 9:8 = AP2 bits */             \
     (((ap1) & 0x3) << 6) |       /* bits 7:6 = AP1 bits */             \
     (((ap0) & 0x3) << 4) |       /* bits 5:4 = AP0 bits */             \
     L2_TYPE_SMALL)               /* bits 1:0 = 0b10 for small page */

#define MAP_L1(virt_addr) ((virt_addr) >> 20)
#define L2_INDEX(virt_addr) (((virt_addr) >> 12) & 0xFF)
#define COARSE_BASE(l1_entry) ((l1_entry) & 0xFFFFFC00)
#define COARSE_PAGE_BASE(coarse_entry) ((coarse_entry) & 0xFFFFF000)

#define DOMAIN_KERNEL 0
#define DOMAIN_USER 1
#define DOMAIN_HW 2
#define DOMAIN_MIXED 3
#define DOMAIN_NO_ACCESS 15

extern volatile uint32_t l1_page_table[NUM_L1_ENTRIES];
extern volatile uint32_t coarse_pt0[NUM_L2_ENTRIES];

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

#define AP_USER_NONE 1
#define AP_USER_READ 2
#define AP_USER_RW 3

void init_page_tables(void);

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
void map_page_user(uintptr_t coarse_pt_phys, uintptr_t va, uintptr_t page_phys, uint8_t ap);

void set_l1_entry(uintptr_t va, uint32_t entry);
uintptr_t vm_alloc(uintptr_t base_va, size_t num_bytes);
void vm_free(uintptr_t va);

#endif
