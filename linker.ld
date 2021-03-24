/*OUTPUT_FORMAT(binary)*/

ENTRY(_start)

SECTIONS
{
/*	. = 0xffffffff80000000; */
	. = 0x0000000000000000;
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

	.preinit_array     :
	{
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	}
	
	.init_array     :
	{
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array ))
		PROVIDE_HIDDEN (__init_array_end = .);
	}
	
	.fini_array     :
	{
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT(.fini_array.*)))
		KEEP (*(.fini_array ))
		PROVIDE_HIDDEN (__fini_array_end = .);
	}
	
	/* Include the list of initialization functions sorted. */
	/*.init_array :
	{
		crti.o(.init_array)
		KEEP (*(SORT(EXCLUDE_FILE(crti.o crtn.o) .init_array.*)))
		KEEP (*(EXCLUDE_FILE(crti.o crtn.o) .init_array))
		crtn.o(.init_array)
	}*/

	/* Include the list of termination functions sorted. */
	/*.fini_array :
	{
		crti.o(.fini_array)
		KEEP (*(SORT(EXCLUDE_FILE(crti.o crtn.o) .fini_array.*)))
		KEEP (*(EXCLUDE_FILE(crti.o crtn.o) .fini_array))
		crtn.o(.fini_array)
	}*/

	_code_end = .;
	. = ALIGN(0x1000); 
	.bss : { *(.bss) }

	. = ALIGN(0x1000); 
	_data_end = .;
}
