#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>

#define CONCAT(a, b) a##b
#define CONCAT2(a, b) CONCAT(a, b)

#define RESERVE_U32(n) uint32_t CONCAT2(_pad_, __LINE__)[n]
#define RESERVE_U8(n) uint8_t CONCAT2(_pad_, __LINE__)[n]

// Linker symbols
extern uint32_t _kernel_heap_start;
extern uint32_t _kernel_heap_end;
extern uint32_t _kernel_stack_bottom;
extern uint32_t _kernel_stack_top;
extern uint32_t _irq_stack_bottom;
extern uint32_t _irq_stack_top;
extern uint32_t _svc_stack_bottom;
extern uint32_t _svc_stack_top;
extern uint32_t _l1pagetable_start;
extern uint32_t _l1pagetable_end;
extern uint32_t _coarsept0_start;
extern uint32_t _coarsept0_end;
extern uint32_t _free_pages_start;
extern uint32_t _free_pages_end;
extern uint32_t _kernel_end;

#endif