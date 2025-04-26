.section .text
.global context_switch
context_switch:
    // Arguments:
    //   r0 = &old_sp
    //   r1 = &new_sp

    // Save current task context
    stmdb sp!, {r4-r11, pc, lr}   // Save callee-saved regs and lr (which holds return address)
    str sp, [r0]              // Save updated SP into *old_sp

    // Load next task context
    ldr sp, [r1]              // Load new task's SP
    ldmia sp!, {r4-r11, lr}   // Restore r4–r11 and jump to lr
    bx lr

    // We never reach here
