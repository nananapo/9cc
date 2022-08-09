	.file	"global3.c"
	.option nopic
	.text
	.globl	g_a
	.section	.rodata
	.align	3
.LC0:
	.string	"HelloWorld"
	.section	.sdata,"aw"
	.align	3
	.type	g_a, @object
	.size	g_a, 8
g_a:
	.dword	.LC0
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-16
	sd	ra,8(sp)
	sd	s0,0(sp)
	addi	s0,sp,16
	lui	a5,%hi(g_a)
	ld	a5,%lo(g_a)(a5)
	mv	a0,a5
	call	puts
	li	a5,0
	mv	a0,a5
	ld	ra,8(sp)
	ld	s0,0(sp)
	addi	sp,sp,16
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.1 20191120 (Red Hat 9.2.1-2)"
	.section	.note.GNU-stack,"",@progbits
