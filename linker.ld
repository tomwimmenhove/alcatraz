OUTPUT_FORMAT(binary)

ENTRY(_start)

SECTIONS
{
/*	. = 0xffffffff80000000; */
	. = 0x0000000000001000;
	.start : { *(.start) }
	.text : { *(.text*) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	_code_start = .;

	.init : { *(.init) }
	.text : { *(.text) }
	.fini : { *(.fini) }

	. = ALIGN(8); 
	.data : { *(.data) }

	. = ALIGN(8); 
	.rodata  : { *(.rodata*) }
	.rodata1 : { *(.rodata1) }
	.tdat    : { *(.tdata .tdata.* .gnu.linkonce.td.*) }
	.tbss    : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }

	.ctors   :
	{
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)
	}
	  
	.dtors   :
	{
		*crtbegin.o(.dtors)
		*crtbegin?.o(.dtors)
		*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors)
		*(SORT(.dtors.*))
		*(.dtors)
	}

	. = ALIGN(0x1000); 
	.bss : { *(.bss) }

	. = ALIGN(0x1000); 
	_data_end = .;
}
