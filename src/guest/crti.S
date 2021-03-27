.macro prologue f s
.section \s
.global \f
.type \f, @function
\f\():
	push %rbp
	movq %rsp, %rbp
.endm

	prologue _init .init
	prologue _fini .fini
