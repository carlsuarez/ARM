#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include "hw/pl011.h"
#include "hw/pl181.h"
#include "hw/pic.h"
#include "hw/timer.h"

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
     ((ap3) << 10) |              /* bits 11:10 = AP3 bits */           \
     ((ap2) << 8) |               /* bits 9:8 = AP2 bits */             \
     ((ap1) << 6) |               /* bits 7:6 = AP1 bits */             \
     ((ap0) << 4) |               /* bits 5:4 = AP0 bits */             \
     L2_TYPE_SMALL)               /* bits 1:0 = 0b10 for small page */

#define DOMAIN_KERNEL 0
#define DOMAIN_USER 1
#define DOMAIN_HW 2
#define DOMAIN_MIXED 3
#define DOMAIN_NO_ACCESS 15

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

extern char _l1_page_table_start;
extern char _coarse_pt0_start;

extern char _buddy_pool_start;
extern char _buddy_pool_end;

extern char _kernel_end;

void init_page_tables(void);

#endif
