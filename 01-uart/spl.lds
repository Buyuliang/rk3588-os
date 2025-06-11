SECTIONS {
	. = 0x00000000;
	. = ALIGN(8);
    .text : 
	{ 
		start.o(.text)
		*(.text) 
	}
    .rodata ALIGN(8) : {*(.rodata*)} 
    .data   ALIGN(8) : { *(.data) }
    
    . = ALIGN(8);
    __bss_start = .;
    .bss   ALIGN(8)  : { *(.bss)  *(COMMON) }
    __bss_end = .;
}
