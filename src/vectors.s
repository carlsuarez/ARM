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
    sub     lr, lr, #4             // Adjust return address
    push    {r0-r12, lr}           // Save all registers and LR
    
    bl      irq_handler_c          // Call C handler directly without mode switches
    
    pop     {r0-r12, lr}           // Restore registers
    movs    pc, lr                 // Return from interrupt (using MOVS to restore CPSR)


// Default dummy handlers
undefined_handler: b .
svc_handler:       b .
prefetch_abort_handler: b .
data_abort_handler: b .
fiq_handler:       b .
