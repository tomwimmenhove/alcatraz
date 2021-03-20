.macro epilogue s
.section \s
	popq %rbp
	ret
.endm

	epilogue .init
	epilogue .fini

