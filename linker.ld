ENTRY(_vectors)

MEMORY
{
    ROM (rx)  : ORIGIN = 0x00000000, LENGTH = 32K
    RAM (rwx) : ORIGIN = 0x00008000, LENGTH = 2M
}

SECTIONS
{
    /* Vector table at 0x00000000 (ROM region) */
    . = ORIGIN(ROM);
    .vectors : {
        KEEP(*(.vectors))
    } > ROM

    /* Main kernel code/data starts at 0x8000 */
    . = ORIGIN(RAM);

    .text : ALIGN(4) {
        _text_start = .;
        *(.text)
        *(.text.*)
        _text_end = .;
    } > RAM

    .rodata : ALIGN(4) {
        _rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        _rodata_end = .;
    } > RAM

    .data : ALIGN(4) {
        _data_start = .;
        *(.data)
        *(.data.*)
        _data_end = .;
    } > RAM
    _data_load = LOADADDR(.data);

    .bss : ALIGN(4) {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        _bss_end = .;
    } > RAM

    /* Heap section (128KB) */
    . = ALIGN(4096);
    .kheap : {
        _kernel_heap_start = .;
        KEEP(*(.kheap))
        . = . + 0x20000; /* 128 KB */
        _kernel_heap_end = .;
    } > RAM


    . = ALIGN(4096);
    .stack : {
        /* Kernel stack (8KB) */
        _kernel_stack_bottom = .;
        KEEP(*(.stack))
        . = . + 0x2000; /* 8 KB */
        _kernel_stack_top = .;

        /* IRQ stack (1KB) */
        . = ALIGN(4096);
        _irq_stack_bottom = .;
        . = . + 0x400;
        _irq_stack_top = .;

        /* SVC stack (1KB) */
        . = ALIGN(4096);
        _svc_stack_bottom = .;
        . = . + 0x400;
        _svc_stack_top = .;
    } > RAM

    /* MMU L1 page table (16KB aligned) */
    . = ALIGN(16384);
    .l1pagetable : {
        _l1pagetable_start = .;
        KEEP(*(.l1pagetable))
        . = . + 0x4000; /* 16KB space */
        _l1pagetable_end = .;
    } > RAM

    /* L2 coarse table (1KB aligned) */
    . = ALIGN(1024);
    _coarsept0_start = .;
    .coarsept0 : {
        KEEP(*(.coarsept0))
    } > RAM
    _coarsept0_end = .;

    /* 1 MB Buddy Allocator Pool */
    . = ALIGN(0x100000);
    _free_pages_start = .;
    . = . + 0x100000;
    _free_pages_end = .;

    /* End of kernel */
    _kernel_end = .;

    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
        *(.comment)
        *(.debug_gdb_scripts*)
    }
}