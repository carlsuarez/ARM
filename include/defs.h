#ifndef DEFS_H
#define DEFS_H

#define CONCAT(a, b) a##b
#define CONCAT2(a, b) CONCAT(a, b)

#define RESERVE_U32(n) uint32_t CONCAT2(_pad_, __LINE__)[n]
#define RESERVE_U8(n) uint8_t CONCAT2(_pad_, __LINE__)[n]

// Linker symbols
extern char _kernel_heap_start;
extern char _kernel_heap_end;
extern char _kernel_stack_bottom;
extern char _kernel_stack_top;
extern char _irq_stack_bottom;
extern char _irq_stack_top;
extern char _svc_stack_bottom;
extern char _svc_stack_top;
extern char _l1_page_table_start;
extern char _l1_page_table_end;
extern char _coarse_pt0_start;
extern char _coarse_pt0_end;
extern char _free_pages_start;
extern char _free_pages_end;
extern char _kernel_end;

#endif