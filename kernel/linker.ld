ENTRY(_vectors)

MEMORY
{
    ROM (rx)  : ORIGIN = 0x00000000, LENGTH = 32K
    RAM (rwx) : ORIGIN = 0x00008000, LENGTH = 10M
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

    /* Heap section (256KB) */
    . = ALIGN(4);
    .kheap : {
        _kernel_heap_start = .;
        KEEP(*(.kheap))
        . = . + 0x40000; /* 256 KB */
        _kernel_heap_end = .;
    } > RAM


    . = ALIGN(4);
    .stack : {
        /* Kernel stack (32KB) */
        _kernel_stack_bottom = .;
        KEEP(*(.stack))
        . = . + 0x8000; /* 32 KB */
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
    . = ALIGN(0x4000);
    .l1pagetable : {
        _l1pagetable_start = .;
        KEEP(*(.l1pagetable))
        . = . + 0x4000; /* 16KB space */
        _l1pagetable_end = .;
    } > RAM
    
    /* 1 MB for coarse page tables */
    . = ALIGN(0x100000);
    .coarsepagetables_space : {
        _coarsepagetables_space_start = .;
        KEEP(*(.coarsepagetables_space))
        . = . + 0x100000; /* 1MB reserved for coarse page tables */
        _coarsepagetables_space_end = .;
    } > RAM

    . = ALIGN(0x4000);
    .l1pagetables_space : {
        _l1pagetables_space_start = .;
        KEEP(*(.l1pagetables_space))
        . = . + 0x200000; /* 2MB reserved for l1 page tables */
        _l1pagetables_space_end = .;
    } > RAM
    
    /* 4 MB for page allocator */
    . = ALIGN(0x1000);
    _free_pages_start = .;
    .freepages : {
        KEEP(*(.freepages))
        . = . + 0x400000; 
    } > RAM
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