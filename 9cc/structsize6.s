	.file	"structsize6.c"
	.option nopic
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-1600
	sd	ra,1592(sp)
	sd	s0,1584(sp)
	addi	s0,sp,1600
	li	t1,-4096
	add	sp,sp,t1
	li	a0,5
	call	pint
	call	pline
	li	a0,10
	call	pint
	call	pline
	li	a0,24
	call	pint
	call	pline
	li	a0,50
	call	pint
	call	pline
	li	a5,4096
	addi	a0,a5,-1346
	call	pint
	call	pline
	li	a5,4096
	addi	a0,a5,-1272
	call	pint
	call	pline
	li	a5,0
	mv	a0,a5
	li	t1,4096
	add	sp,sp,t1
	ld	ra,1592(sp)
	ld	s0,1584(sp)
	addi	sp,sp,1600
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.1 20191120 (Red Hat 9.2.1-2)"
	.section	.note.GNU-stack,"",@progbits
