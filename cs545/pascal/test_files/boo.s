	.text
.globl _main
_main:
	leal 4(%esp), %ecx
	andl $-16, %esp
	pushl -4(%ecx)
	pushl %ebp
	movl %esp, %ebp
	pushl %ecx
	subl $4, %esp
	call _foo
	addl $4, %esp
	popl %ecx
	popl %ebp
	leal -4(%ecx), %esp
	ret
.LC0:
	.ascii "int: %d\12\0"
