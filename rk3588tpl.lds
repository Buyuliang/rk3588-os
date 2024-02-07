SECTIONS {
	. = 0xfdcc1000;
	. = ALIGN(8);
    .text : 
	{ 
		rk3588tpl_start.o(.text)
		*(.text) 
	}
    .rodata ALIGN(8) : {*(.rodata*)} 
    .data   ALIGN(8) : { *(.data) }
    
    . = ALIGN(8);
    __bss_start = .;
    .bss   ALIGN(8)  : { *(.bss)  *(COMMON) }
    __bss_end = .;
}
