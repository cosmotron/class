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
	.text
.globl _testadd
_testadd:
	pushl %ebp
	movl %esp, %ebp
	subl $12, %esp
	movl $5, %ebx	// <<< expr_gencode case 0 (int)
	movl %ebx, -12(%ebp)	// move value into variable (local)
	movl 8(%ebp), %ebx	// <<< expr_gencode case 0 (id)
	addl 12(%ebp), %ebx	// <<< expr_gencode case 1 (id)
	movl %ebx, -20(%ebp)	// move value into variable (local)
	movl %ebx, %eax
	leave
	ret
	.text
.globl _foo
_foo:
	pushl %ebp
	movl %esp, %ebp
	subl $24, %esp
	movl $5, %ebx	// <<< expr_gencode case 0 (int)
// >>> func call
	pushl %ebx
	pushl %ecx
	pushl %edx
	movl $7, %ebx	// <<< expr_gencode case 0 (int)
	subl $3, %ebx	// <<< expr_gencode case 1 (int)
	movl %ebx, (%esp)	// (f/p call gencode) set up arg on stack
	movl $3, %ebx	// <<< expr_gencode case 0 (int)
	imull $5, %ebx	// <<< expr_gencode case 1 (int)
	movl %ebx, 4(%esp)	// (f/p call gencode) set up arg on stack
	call _testadd
	popl %edx
	popl %ecx
	popl %ebx
// ### func call as a child, look in eax
	addl %eax, %ebx	// <<< expr_gencode case 3
	addl $6, %ebx	// <<< expr_gencode case 1 (int)
	imull $2, %ebx	// <<< expr_gencode case 1 (int)
	movl %ebx, -8(%ebp)	// move value into variable (local)
// >>> proc call
	movl -8(%ebp), %ebx	// (f/p call gencode) arg is id
	movl %ebx, 4(%esp)	// (f/p call gencode) set up arg on stack
	movl $.LC0, (%esp)
	call _printf
	leave
	ret

