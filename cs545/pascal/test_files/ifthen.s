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
.globl _foo
_foo:
	pushl %ebp
	movl %esp, %ebp
	subl $24, %esp
	movl $5, %ebx	// <<< expr_gencode case 0 (int)
	movl %ebx, -8(%ebp)	// move value into variable (local)
	movl $3, %ebx	// <<< expr_gencode case 0 (int)
	movl %ebx, -12(%ebp)	// move value into variable (local)
	movl $10, %ebx	// <<< expr_gencode case 0 (int)
	movl %ebx, -16(%ebp)	// move value into variable (local)
	movl $1, %ebx	// <<< (if then gencode) handle int
	movl %ebx, %edx	// (if then gencode) move expr left to edx
	pushl %edx	// (if then gencode) save edx for cmpl
	movl -8(%ebp), %ebx	// <<< (if then gencode) handle id
	movl %ebx, %eax	// (if then gencode) move expr right to eax
	popl %edx	// (if then gencode) restore edx for cmpl
	cmpl %eax, %edx	// (if then gencode) comparison
	jl .L1	// (if then gencode) jump to THEN
	jmp .L2
// begin THEN
.L1:
	movl $3, %ebx	// <<< (if then gencode) handle int
	movl %ebx, %edx	// (if then gencode) move expr left to edx
	pushl %edx	// (if then gencode) save edx for cmpl
	movl -12(%ebp), %ebx	// <<< (if then gencode) handle id
	movl %ebx, %eax	// (if then gencode) move expr right to eax
	popl %edx	// (if then gencode) restore edx for cmpl
	cmpl %eax, %edx	// (if then gencode) comparison
	je .L3	// (if then gencode) jump to THEN
	jmp .L4
// begin THEN
.L3:
	movl $1, %ebx	// <<< expr_gencode case 0 (int)
	movl %ebx, -16(%ebp)	// move value into variable (local)
	jmp .L5
// end THEN
// begin ELSE
.L4:
	movl $2, %ebx	// <<< expr_gencode case 0 (int)
	movl %ebx, -16(%ebp)	// move value into variable (local)
// end ELSE
.L5:
	jmp .L2
// end THEN
.L2:
// >>> proc call
	movl -16(%ebp), %ebx	// (f/p call gencode) arg is id
	movl %ebx, 4(%esp)	// (f/p call gencode) set up arg on stack
	movl $.LC0, (%esp)
	call _printf
	leave
	ret

