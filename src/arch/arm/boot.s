.section .l1pagetable, "aw", %progbits
.align 14

.global l1_page_table
l1_page_table:
    .space 16384

.section .coarsept0, "aw", %progbits
.align 

.global coarse_pt0
coarse_pt0:
    .space 1024

.section .text
.global _start

_start:
    /* Copy .data from LMA (ROM) to VMA (RAM) */
    ldr r0, =_data_load     /* ROM source */
    ldr r1, =_data_start    /* RAM destination */
    ldr r2, =_data_end
copy_data:
    cmp r1, r2
    ldrlt r3, [r0], #4
    strlt r3, [r1], #4
    blt copy_data

    /* Zero out the .bss section */
    ldr r0, =_bss_start
    ldr r1, =_bss_end
    mov r2, #0
zero_bss:
    cmp r0, r1
    strlt r2, [r0], #4
    blt zero_bss

    /* Set up IRQ mode and stack */
    mrs r0, cpsr
    bic r0, r0, #0x1F        /* Clear mode bits */
    orr r0, r0, #0x12        /* Set IRQ mode (0b10010) */
    msr cpsr_c, r0

    ldr sp, =_irq_stack_top  /* IRQ stack grows down from top */

    /* Set up Supervisor mode and stack */
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #0x13        /* SVC mode (0b10011) */
    msr cpsr_c, r0
    
    ldr sp, =_svc_stack_top

    /* Set up System mode and main stack */
    mrs r0, cpsr
    bic r0, r0, #0x1F
    orr r0, r0, #0x1F        /* System mode (0b11111) */
    msr cpsr_c, r0

    ldr sp, =_kernel_stack_top      /* Regular task stack */

    bl init_page_tables
    b kernel
    ldr r0, =l1_page_table
    
    // Invalidate TLB (TLBIALL)
    mov     r1, #0
    mcr     p15, 0, r1, c8, c7, 0

    // Set TTBR0 with L1 table base (in r0)
    mcr     p15, 0, r0, c2, c0, 0

    // Optional: Set TTBCR to 0 (not needed on ARMv5, but safe)
    mov     r1, #0
    mcr     p15, 0, r1, c2, c0, 2

    // Set Domain Access Control Register
    /* 
    Domain 0: Kernel domain, check permissions
    Domain 1: User domain, do not check permissions
    Domain 2: Hardware, check permissions
    Domain 3: Mixed kernel and user, check permissions
    Domains[5-15]: No access
    */
    mov r1, #0xFFFFFFFF
    mcr  p15, 0, r1, c3, c0, 0  @ Write to DACR


    // Data Synchronization Barrier (DSB alternative)
    mov     r1, #0
    mcr     p15, 0, r1, c7, c10, 4

    // Read SCTLR into r1
    mrc     p15, 0, r1, c1, c0, 0

    // Set bits: MMU (bit 0), D-cache (bit 2), I-cache (bit 12)
    orr     r1, r1, #(1 << 0)   // MMU enable
    orr     r1, r1, #(1 << 2)   // D-cache enable
    orr     r1, r1, #(1 << 12)  // I-cache enable

    // Write back modified SCTLR
    mcr     p15, 0, r1, c1, c0, 0

    // Prefetch Flush (ISB alternative)
    mov     r1, #0
    mcr     p15, 0, r1, c7, c5, 4

kernel:
    /* Jump to main kernel entry */
    bl kernel_main

halt:
    b halt
