SECTIONS
{
    . = 0x800000;

    .text : {
        *(.text)
        *(.text.*)
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    .data : {
        *(.data)
        *(.data.*)
    }

    .bss : {
        *(.bss COMMON)
    }

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame)
    }
}
