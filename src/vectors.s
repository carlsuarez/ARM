.section .vectors, "a"
.global _vectors

_vectors:
    b _start                // Reset
    b undefined_handler     // Undefined instruction
    b swi_handler           // Software interrupt
    b prefetch_abort_handler // Prefetch abort
    b data_abort_handler    // Data abort
    b .                     // Reserved
    b irq_handler           // IRQ
    b fiq_handler           // FIQ

irq_handler:
    sub     lr, lr, #4             // construct the return address
    push    {lr}                   // and push the adjusted lr_IRQ
    mrs     lr, SPSR               // copy spsr_IRQ to lr
    push    {R0-R4,R12,lr}         // save AAPCS regs and spsr_IRQ
    bl      identify_and_clear_source
    msr     CPSR_c, #0x9F          // switch to SYS mode, IRQ is
                                   // still disabled. USR mode
                                   // registers are now current.
    and     R1, sp, #4             // test alignment of the stack
    sub     sp, sp, R1             // remove any misalignment (0 or 4)
    push    {R1,lr}                // store the adjustment and lr_USR
    msr     CPSR_c, #0x1F          // enable IRQ
    bl      irq_handler_c
    msr     CPSR_c, #0x9F          // disable IRQ, remain in SYS mode
    pop     {R1,lr}                // restore stack adjustment and lr_USR
    add     sp, sp, R1             // add the stack adjustment (0 or 4)
    msr     CPSR_c, #0x92          // switch to IRQ mode and keep IRQ
                                   // disabled. FIQ is still enabled.
    pop     {R0-R4,R12,lr}         // restore registers and
    msr     SPSR_cxsf, lr          // spsr_IRQ
    LDM     sp!, {pc}^             // return from IRQ.


// Default dummy handlers
undefined_handler: b .
swi_handler:       b .
prefetch_abort_handler: b .
data_abort_handler: b .
fiq_handler:       b .
