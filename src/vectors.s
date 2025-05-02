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
    /* Adjust LR_irq to point to the interrupted instruction */
    sub     lr, lr, #4

    push {lr}               /* Save LR_irq */

    /* Save general-purpose registers r0–r12 */
    push    {r0-r12}

    /* Save SPSR_irq and LR_irq */
    mrs     r2, spsr           /* r2 = SPSR_irq */
    push    {r2}              /* Save SPSR_irq */

    /* Disable IRQs to prevent nested interrupts */
    mrs     r0, cpsr
    orr     r0, r0, #0x80
    msr     cpsr_c, r0

    /* Save original mode's LR and SP */
    mrs     r5, cpsr              /* r5 = CPSR (IRQ mode) */
    mov     r4, r5                /* Keep a copy of IRQ CPSR */
    and     r3, r2, #0x1F         /* r3 = mode bits from SPSR */
    bic     r5, r5, #0x1F
    orr     r5, r5, r3
    msr     cpsr_c, r5            /* Switch to original mode */

    mov     r6, lr                /* r6 = original LR (return address) */
    mov     r7, sp                /* r7 = original SP */

    msr     cpsr_c, r4            /* Switch back to IRQ mode */

    /* Push original LR and SP */
    push    {r6, r7}              /* LR_orig, SP_orig */

    /* === Save context to task's stack === */
    ldr     r0, =current
    ldr     r1, [r0]              /* r1 = current task struct */
    ldr     r3, [r1]              /* r3 = task->sp */

    /* Save IRQ stack context to task's stack (17 registers) */
    mov     r0, sp
    mov     r2, #(17 * 4)
    sub     r3, r3, r2
    mov     r8, r3                /* Save new task SP */

copy_to_task_stack:
    ldr     r4, [r0], #4
    str     r4, [r3], #4
    subs    r2, r2, #4
    bne     copy_to_task_stack

    str     r8, [r1]              /* Update task->sp */
    add     sp, sp, #(17 * 4)     /* Clear IRQ stack */

    /* === Call high-level C IRQ handler === */
    bl      irq_handler_c

restore_context:
    /* === Restore context from task's stack === */
    ldr     r0, =current
    ldr     r1, [r0]
    ldr     r3, [r1]              /* r3 = saved task SP */

    sub     sp, sp, #(17 * 4)
    mov     r0, sp
    mov     r2, #(17 * 4)

copy_from_task_stack:
    ldr     r4, [r3], #4
    str     r4, [r0], #4
    subs    r2, r2, #4
    bne     copy_from_task_stack

    str     r3, [r1]              /* Update task->sp */

    /* === Restore in reverse order === */
    pop     {r6, r7}              /* r6 = LR_orig, r7 = SP_orig */
    pop     {r2}                  /* r2 = SPSR */
    msr     spsr_cxsf, r2

    /* Restore original mode's SP and LR */
    mrs     r0, cpsr           /* Read current CPSR */
    bic     r0, r0, #0x1F      /* Clear current mode bits */
    orr     r0, r0, #0x1F      /* Set System mode (0b11111) */
    msr     cpsr_c, r0         /* Write back to CPSR */

    mov     sp, r7
    mov     lr, r6

    /* Back to IRQ mode */
    mrs     r0, cpsr           /* Read current CPSR */
    bic     r0, r0, #0x9F      /* Clear current mode bits and enable IRQ */
    orr     r0, r0, #0x12      /* Set System mode (0b10010) */
    msr     cpsr_c, r0         /* Write back to CPSR */

    /* Restore general-purpose registers */
    pop     {r0-r12}

    /* Restore PC from LR_irq */
    pop     {pc}                  /* This is lr_irq → pc */




// Default dummy handlers
undefined_handler: b .
svc_handler:       b .
prefetch_abort_handler: b .
data_abort_handler: b .
fiq_handler:       b .
