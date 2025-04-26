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
irq_handler:
    // Disable IRQs
    mrs     r0, cpsr
    orr     r0, r0, #0x80      // Set I bit to disable IRQs
    msr     cpsr, r0

    sub     lr, lr, #4             // Adjust return address
    push    {r0-r12, lr}           // Save all registers and LR
    
    bl      irq_handler_c          // Call C handler directly without mode switches
    
    // Enable IRQs again
    mrs     r0, cpsr
    bic     r0, r0, #0x80         // Clear I bit to enable IRQs
    msr     cpsr, r0
    
    pop     {r0-r12, lr}           // Restore registers
    movs    pc, lr                 // Return from interrupt (using MOVS to restore CPSR)


// Default dummy handlers
undefined_handler: b .
svc_handler:       b .
prefetch_abort_handler: b .
data_abort_handler: b .
fiq_handler:       b .
