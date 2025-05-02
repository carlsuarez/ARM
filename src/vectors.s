.section .vectors, "ax"
.global _vectors

_vectors:
    b _start                // Reset
    b undefined_handler     // Undefined instruction
    b svc_handler           // Software interrupt
    b prefetch_abort_handler // Prefetch abort
    b data_abort_handler    // Data abort
    nop                     // Reserved
    b irq_handler           // IRQ
    b fiq_handler           // FIQ

.section .text
.extern current
.extern irq_handler_c

/*
 * ARM Interrupt Request (IRQ) Handler
 * Handles context saving/restoring during interrupts
 */
irq_handler:
    sub     lr, lr, #4          /* Adjust LR_irq to interrupted PC */
    push    {lr}                /* Save LR_irq */

    push    {r0-r12}            /* Save general-purpose registers */

    mrs     r2, spsr            /* r2 = SPSR_irq */
    push    {r2}                /* Save SPSR_irq */

    /* Disable IRQs and FIQs during handler */
    mrs     r0, cpsr
    orr     r0, r0, #0xC0
    msr     cpsr_c, r0

    /* Switch to the original mode to get SP and LR */
    mrs     r5, cpsr            /* r5 = CPSR in IRQ mode */
    mov     r4, r5              /* Save CPSR for restoring later */
    and     r3, r2, #0x1F       /* r3 = original mode */
    bic     r5, r5, #0x1F
    orr     r5, r5, r3
    msr     cpsr_c, r5          /* Switch to original mode */

    mov     r6, lr              /* r6 = original LR */
    mov     r7, sp              /* r7 = original SP */

    msr     cpsr_c, r4          /* Back to IRQ mode */
    push    {r6, r7}            /* Save original LR and SP */

    /* Check if current task is TERMINATED */
    ldr     r0, =current
    ldr     r1, [r0]            /* r1 = current task */
    ldr     r2, [r1, #4]  /* r2 = task->state */
    cmp     r2, #3              /* Check if task is TERMINATED */
    beq     skip_save_context

    /* === Save context to task's stack === */
    ldr     r3, [r1]            /* r3 = task->sp */

    mov     r0, sp
    mov     r2, #(17 * 4)
    sub     r3, r3, r2
    mov     r8, r3              /* New SP for task */

copy_to_task_stack:
    ldr     r4, [r0], #4
    str     r4, [r3], #4
    subs    r2, r2, #4
    bne     copy_to_task_stack

    str     r8, [r1]            /* task->sp = new SP */
    add     sp, sp, #(17 * 4)   /* Clear IRQ stack */

skip_save_context:
    bl      irq_handler_c       /* Schedule next task */

    /* === Restore context from task's stack === */
    ldr     r0, =current
    ldr     r1, [r0]
    ldr     r3, [r1]            /* r3 = task->sp */

    sub     sp, sp, #(17 * 4)
    mov     r0, sp
    mov     r2, #(17 * 4)

copy_from_task_stack:
    ldr     r4, [r3], #4
    str     r4, [r0], #4
    subs    r2, r2, #4
    bne     copy_from_task_stack

    str     r3, [r1]            /* Update task->sp */

    /* === Restore state === */
    pop     {r6, r7}            /* r6 = LR_orig, r7 = SP_orig */
    pop     {r2}                /* r2 = SPSR_orig */
    msr     spsr_cxsf, r2

    /* Switch to System mode to restore SP and LR */
    mrs     r0, cpsr
    bic     r0, r0, #0x1F
    orr     r0, r0, #0x1F       /* System mode (0b11111) */
    msr     cpsr_c, r0

    mov     sp, r7
    mov     lr, r6

    /* Return to IRQ mode */
    mrs     r0, cpsr
    bic     r0, r0, #0xDF       /* Enable IRQ and FRQ */
    orr     r0, r0, #0x12       /* IRQ mode (0b10010) */
    msr     cpsr_c, r0

    pop     {r0-r12}
    pop     {pc}                /* Return from IRQ */
    


// Default dummy handlers
undefined_handler: b .
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
