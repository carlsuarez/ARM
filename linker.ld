ENTRY(_start)

SECTIONS {
    . = 0x00000000;

    .vectors : {
        KEEP(*(.vectors))
    }

    . = 0x8000;

    .text : {
        *(.text*)
    }

    .rodata : {
        *(.rodata*)
    }

    /* .data section copied from LMA (ROM) to VMA (RAM) */
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

    . = ALIGN(4K);
    .stack : {
        _stack_top = .;
        . = . + 0x1000;
        _stack_bottom = .;
    }

    /* Load address for .data in ROM */
    _data_load = LOADADDR(.data);

    /DISCARD/ : {
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }
}