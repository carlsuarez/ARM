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

    ldr sp, =_stack_top      /* Regular task stack */

    /* Jump to main kernel entry */
    bl kernel_main

halt:
    b halt
