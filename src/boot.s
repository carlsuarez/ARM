.global _start
.section .text

_start:
    ldr sp, =_stack_top     // Set up stack pointer
    bl main                 // Call C main function
    b .                     // Infinite loop after main
