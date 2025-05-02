ENTRY(_vectors)

SECTIONS {
    /* Vector table at address 0x00000000 */
    . = 0x00000000;

    .vectors : {
        KEEP(*(.vectors))
    }

    /* Kernel code starts at 0x8000 */
    . = 0x8000;

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    .data : AT(ADDR(.rodata) + SIZEOF(.rodata)) {
        _data_start = .;
        *(.data*)
        _data_end = .;
    }

    .bss : {
        _bss_start = .;
        *(.bss*)
        _bss_end = .;
    }

    /* Heap (aligned to 4KB, 64 KB total) */
    . = ALIGN(4096);
    _heap_start = .;
    . = . + 0x10000; /* 64 KB */
    _heap_end = .;

    /* Kernel Stack (4KB) */
    . = ALIGN(4096);
    _stack_bottom = .;
    . = . + 0x1000;
    _stack_top = .;

    /* IRQ Stack (1KB) */
    . = ALIGN(4096);
    _irq_stack_bottom = .;
    . = . + 0x400;
    _irq_stack_top = .;

    /* SVC Stack (1KB) */
    . = ALIGN(4096);
    _svc_stack_bottom = .;
    . = . + 0x400;
    _svc_stack_top = .;

    /* For data relocation (copying from flash to RAM) */
    _data_load = LOADADDR(.data);

    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }
}
