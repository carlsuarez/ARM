.section .vectors, "ax"
.global _vectors
.align 2

_vectors:
    b _start                // Reset
    b undefined_handler     // Undefined instruction
    b svc_handler           // Software interrupt
    b prefetch_abort_handler // Prefetch abort
    b data_abort_handler    // Data abort
    nop                     // Reserved
    b irq_handler           // IRQ
    b fiq_handler           // FIQ

.extern current
.extern irq_handler_c

/*
 * ARM Interrupt Request (IRQ) Handler
 * Uses context buffer in PCB structure for reliable context switching
 */

irq_handler:
    sub     lr, lr, #4          /* Adjust LR_irq to interrupted PC */
    
    /* === Save all context to IRQ stack first === */
    push    {r0-r12, lr}        /* Save all GPRs and adjusted LR */
    mrs     r0, spsr            
    push    {r0}                /* Save SPSR_irq */
    
    /* Get original mode SP and LR while we can still access them */
    and     r1, r0, #0x1F       /* r1 = original mode from SPSR */
    mrs     r2, cpsr            /* r2 = current CPSR (IRQ mode) */
    bic     r3, r2, #0x1F       /* Clear mode bits */
    orr     r3, r3, r1          /* Set original mode */
    msr     cpsr_c, r3          /* Switch to original mode */
    
    mov     r4, sp              /* r4 = original SP */
    mov     r5, lr              /* r5 = original LR */
    
    msr     cpsr_c, r2          /* Back to IRQ mode */
    push    {r4, r5}            /* Save original SP, LR to IRQ stack */
    
    /* Disable IRQs/FIQs for critical section */
    orr     r2, r2, #0xC0
    msr     cpsr_c, r2
    
    /* === Save context to current task's context buffer === */
    ldr     r6, =current
    ldr     r7, [r6]            /* r7 = current task */
    cmp     r7, #0              /* Check if current is NULL */
    beq     call_scheduler
    
    ldr     r8, [r7, #4]        /* r8 = task->state */
    cmp     r8, #3              /* Check if task is TERMINATED */
    beq     call_scheduler
    
    /* Calculate offset to context buffer in PCB */
    add     r8, r7, #8         /* r8 = &task->context_buffer */
    
    /* Copy entire IRQ stack contents to context buffer */
    mov     r9, sp              /* r9 = IRQ stack pointer */
    mov     r10, #17            /* r10 = number of words to copy */
    
save_context_loop:
    ldr     r11, [r9], #4       /* Load from IRQ stack, increment */
    str     r11, [r8], #4       /* Store to context buffer, increment */
    subs    r10, r10, #1
    bne     save_context_loop
    
    /* Clear IRQ stack */
    add     sp, sp, #(17 * 4)

call_scheduler:
    /* Call C scheduler - this changes current and page tables */
    bl      irq_handler_c
    
    /* === Restore context from new task's context buffer === */
    ldr     r6, =current
    ldr     r7, [r6]            /* r7 = new current task */
    
    /* Calculate offset to context buffer */
    add     r8, r7, #8         /* r8 = &task->context_buffer */
    mov     r9, #17            /* r9 = number of words to copy */
    
restore_context_loop:
    ldr     r11, [r8], #4       /* Load from context buffer, increment */
    push    {r11}
    subs    r9, r9, #1
    bne     restore_context_loop
    
    /* === Restore processor state === */
    pop     {r4, r5}            /* r4 = original LR, r5 = original SP */
    pop     {r0}                /* r0 = SPSR */
    msr     spsr_cxsf, r0       /* Restore SPSR */
    
    /* Switch to System mode to restore SP and LR */
    /* Cannot switch directly from IRQ to User mode - use System mode instead */
    mrs     r2, cpsr            /* r2 = current CPSR (IRQ mode) */
    bic     r3, r2, #0x1F       /* Clear mode bits */
    orr     r3, r3, #0x1F       /* Set System mode (0b11111) */
    msr     cpsr_c, r3          /* Switch to System mode */
    
    /* Restore original mode SP and LR */
    mov     lr, r4
    mov     sp, r5
    
    /* Return to IRQ mode for final cleanup */
    bic     r2, r2, #0xC0       /* Re-enable IRQs/FIQs */
    msr     cpsr_c, r2          /* Back to IRQ mode */

    /* Restore all registers and return */
    pop     {r0-r12}            /* Restore general purpose registers */
    pop     {lr}                /* Return to interrupted code */
    subs    pc, lr, #0
    


// Default dummy handlers
undefined_handler: b .

.extern svc_handler_c
svc_handler:
    sub sp, sp, #68     // Make space for 17 registers
    stmia sp, {r0-r12, lr}    // Store r0-r12, lr onto the stack

    mrs r0, spsr              // Get spsr into r0
    str r0, [sp, #64]   // Store spsr at end of frame

    mov r0, sp                // Pass pointer to frame as argument
    bl svc_handler_c          // Call C handler with regs_t*

    ldmia sp, {r0-r12, lr}    // Restore r0-r12, lr
    ldr r1, [sp, #64]   // Restore spsr into r1
    add sp, sp, #68     // Clean up stack
    msr spsr_cxsf, r1         // Write spsr back
    movs pc, lr               // Return from SVC


prefetch_abort_handler: b .
data_abort_handler: b .
fiq_handler:       b .
