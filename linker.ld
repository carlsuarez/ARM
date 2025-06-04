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

    /* --- Kernel Heap (for kmalloc, slab allocators, etc) --- */
    . = ALIGN(4096);
    _kernel_heap_start = .;
    . = . + 0x20000; /* 128 KB */
    _kernel_heap_end = .;

    /* --- Stacks --- */
    . = ALIGN(4096);
    _kernel_stack_bottom = .;
    . = . + 0x2000; /* 8KB */
    _kernel_stack_top = .;

    . = ALIGN(4096);
    _irq_stack_bottom = .;
    . = . + 0x400;  /* 1KB */
    _irq_stack_top = .;

    . = ALIGN(4096);
    _svc_stack_bottom = .;
    . = . + 0x400;  /* 1KB */
    _svc_stack_top = .;

    /* --- MMU Page Tables --- */

    . = ALIGN(16384); /* 16KB alignment for L1 table */
    _l1_page_table_start = .;
    .l1pagetable : {
        KEEP(*(.l1pagetable))  /* Optionally match by section name */
        . = . + 0x4000;        /* 16 KB */
    }
    _l1_page_table_end = .;

    . = ALIGN(1024); /* 1KB alignment for coarse L2 tables */
    _coarse_pt0_start = .;
    .coarsept0 : {
        KEEP(*(.coarsept0))
        . = . + 0x400;         /* 1 KB */
    }
    _coarse_pt0_end = .;
    
    /* --- Buddy Allocator Pool (for user task memory, etc) --- */
    . = ALIGN(0x100000);
    _free_pages_start = .;
    . = . + 0x100000; /* 1 MB */
    _free_pages_end = .;

    /* Final symbol: last used memory address */
    _kernel_end = .;

    /* For data relocation (copying from flash to RAM) */
    _data_load = LOADADDR(.data);
    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }
}
