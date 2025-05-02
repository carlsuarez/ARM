#ifndef INTERUPT_H
#define INTERUPT_H

void irq_handler_c(void);

#define cli()                           \
    do                                  \
    {                                   \
        unsigned int _tmp;              \
        asm volatile(                   \
            "mrs %0, cpsr\n\t"          \
            "bic %0, %0, #(1 << 7)\n\t" \
            "msr cpsr_c, %0\n\t"        \
            : "=r"(_tmp)                \
            :                           \
            : "memory");                \
    } while (0)

#define sei()                           \
    do                                  \
    {                                   \
        unsigned int _tmp;              \
        asm volatile(                   \
            "mrs %0, cpsr\n\t"          \
            "orr %0, %0, #(1 << 7)\n\t" \
            "msr cpsr_c, %0\n\t"        \
            : "=r"(_tmp)                \
            :                           \
            : "memory");                \
    } while (0)

#define clf()                           \
    do                                  \
    {                                   \
        unsigned int _tmp;              \
        asm volatile(                   \
            "mrs %0, cpsr\n\t"          \
            "bic %0, %0, #(1 << 6)\n\t" \
            "msr cpsr_c, %0\n\t"        \
            : "=r"(_tmp)                \
            :                           \
            : "memory");                \
    } while (0)

#define sef()                           \
    do                                  \
    {                                   \
        unsigned int _tmp;              \
        asm volatile(                   \
            "mrs %0, cpsr\n\t"          \
            "orr %0, %0, #(1 << 6)\n\t" \
            "msr cpsr_c, %0\n\t"        \
            : "=r"(_tmp)                \
            :                           \
            : "memory");                \
    } while (0)

#endif