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

irq_handler:
    /* Adjust LR_irq to point to next instruction after the interrupted one */
    sub     lr, lr, #4

    /* Save general-purpose registers to IRQ stack */
    push    {r0-r12}

    /* Save SPSR_irq and LR_irq */
    mrs     r2, spsr              /* r2 = SPSR_irq */
    push    {lr, r2}              /* Save LR_irq and SPSR_irq */

    /* Disable IRQ */
    mrs r0, cpsr
    orr r1, r0, #0x80
    msr cpsr_c, r1


    /* Save original mode's LR (user or system mode) */
    mrs     r5, cpsr              /* r5 = current CPSR (IRQ mode) */
    mov     r4, r5                /* Preserve IRQ CPSR in r4 */

    and     r3, r2, #0x1F         /* r3 = original mode bits from SPSR */
    bic     r5, r5, #0x1F         /* Clear mode bits */
    orr     r5, r5, r3            /* Set mode bits to original mode */
    msr     cpsr_c, r5            /* Switch to original mode */

    mov     r6, lr                /* Save original mode's LR (lr_user) in r6 */
    msr     cpsr_c, r4            /* Switch back to IRQ mode */

    /* Push lr_user to IRQ stack (now total 16 registers saved) */
    push    {r6}

    /* === Save context to task's stack === */
    ldr     r0, =current
    ldr     r1, [r0]              /* r1 = current task struct */
    ldr     r3, [r1]              /* r3 = task->sp */

    mov     r0, sp                /* Source = IRQ SP */
    mov     r2, #(16 * 4)         /* 16 registers: r0–r12, lr_irq, spsr, lr_user */
    sub     r3, r3, r2            /* Pre-adjust task SP */
    mov     r7, r3                /* Save new task SP */

copy_to_task_stack:
    ldr     r4, [r0], #4
    str     r4, [r3], #4
    subs    r2, r2, #4
    bne     copy_to_task_stack

    str     r7, [r1]              /* Update task->sp */

    /* Restore IRQ SP */
    add     sp, sp, #(16 * 4)

    /* === Call C-level handler === */
    bl      irq_handler_c

    /* === Restore task context === */
    ldr     r0, =current
    ldr     r1, [r0]
    ldr     r7, [r1]              /* r7 = task->sp */

    sub     sp, sp, #(16 * 4)
    mov     r0, sp                /* Destination = IRQ SP */
    mov     r3, r7                /* Source = task SP */
    mov     r2, #(16 * 4)

copy_from_task_stack:
    ldr     r4, [r3], #4
    str     r4, [r0], #4
    subs    r2, r2, #4
    bne     copy_from_task_stack

    add     r7, r7, #(16 * 4)
    str     r7, [r1]              /* Update task->sp */

    /* === Begin restoring registers === */
    pop     {r6}                  /* r6 = lr_user */
    pop     {r2}                  /* r2 = spsr_irq */

    /* Restore SPSR BEFORE switching to user mode */
    msr     spsr_cxsf, r2

    /* Switch to original mode */
    mrs     r5, cpsr
    mov     r4, r5
    and     r3, r2, #0x1F
    bic     r5, r5, #0x1F
    orr     r5, r5, r3
    msr     cpsr_c, r5

    /* Enable IRQ */
    mrs r5, cpsr
    bic r5, r5, #0x80
    msr cpsr_c, r5


    mov     sp, r7                /* Restore original SP */
    mov     lr, r6                /* Restore original LR (lr_user) */

    /* Switch back to IRQ mode to clean up */
    msr     cpsr_c, r4

    pop     {lr}                  /* Restore lr_irq */
    pop     {r0-r12}              /* Restore general-purpose registers */

    /* Return from IRQ */
    movs    pc, lr


// Default dummy handlers
undefined_handler: b .
svc_handler:       b .
prefetch_abort_handler: b .
data_abort_handler: b .
fiq_handler:       b .
