	.file	"global2.c"
	.option nopic
	.text
	.globl	g_a
	.section	.sdata,"aw"
	.type	g_a, @object
	.size	g_a, 1
g_a:
	.byte	103
	.section	.rodata
	.align	3
.LC0:
	.string	"%c\n"
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
	lbu	a5,%lo(g_a)(a5)
	sext.w	a5,a5
	mv	a1,a5
	lui	a5,%hi(.LC0)
	addi	a0,a5,%lo(.LC0)
	call	printf
	li	a5,0
	mv	a0,a5
	ld	ra,8(sp)
	ld	s0,0(sp)
	addi	sp,sp,16
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.1 20191120 (Red Hat 9.2.1-2)"
	.section	.note.GNU-stack,"",@progbits
