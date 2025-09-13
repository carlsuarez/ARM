ENTRY(main)

SECTIONS {
    . = 0x1000;
    
    .text : {
        *(.text.startup)
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
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    
    /* Keep dynamic sections */
    .dynamic : {
        *(.dynamic)
    }
    
    .dynsym : {
        *(.dynsym)
    }
    
    .dynstr : {
        *(.dynstr)
    }
    
    .hash : {
        *(.hash)
    }
    
    .got : {
        *(.got)
    }
    
    .got.plt : {
        *(.got.plt)
    }
    
    .rel.dyn : {
        *(.rel.dyn)
    }
    
    .rel.plt : {
        *(.rel.plt)
    }
}