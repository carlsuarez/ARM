#ifndef IRQ_HANDLER_H
#define IRQ_HANDLER_H

void irq_handler_c(void);

#define sei()                                                      \
    __asm__ volatile(                                              \
        "mrs r0, cpsr\n\t"          /* Move CPSR to r0 */          \
        "bic r0, r0, #(1 << 7)\n\t" /* Clear I bit (IRQ enable) */ \
        "msr cpsr_c, r0\n\t"        /* Write back to CPSR */       \
    )

#define cli()                                                     \
    __asm__ volatile(                                             \
        "mrs r0, cpsr\n\t"          /* Move CPSR to r0 */         \
        "orr r0, r0, #(1 << 7)\n\t" /* Set I bit (IRQ disable) */ \
        "msr cpsr_c, r0\n\t"        /* Write back to CPSR */      \
    )

#define sef()                                                      \
    __asm__ volatile(                                              \
        "mrs r0, cpsr\n\t"          /* Move CPSR to r0 */          \
        "bic r0, r0, #(1 << 6)\n\t" /* Clear F bit (FIQ enable) */ \
        "msr cpsr_c, r0\n\t"        /* Write back to CPSR */       \
    )

#define clf()                                                     \
    __asm__ volatile(                                             \
        "mrs r0, cpsr\n\t"          /* Move CPSR to r0 */         \
        "orr r0, r0, #(1 << 6)\n\t" /* Set F bit (FIQ disable) */ \
        "msr cpsr_c, r0\n\t"        /* Write back to CPSR */      \
    )

#endif