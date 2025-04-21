.section .text
.global _start

_start:
    /* Set up the stack */
    ldr sp, =_stack_top

    /* Copy .data from ROM (LMA) to RAM (VMA) */
    ldr r0, =_data_load     /* Source in ROM */
    ldr r1, =_data_start    /* Destination in RAM */
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

    /* Call main kernel function */
    bl kernel_main
    /* Infinite loop after main */
    loop:
        b loop
