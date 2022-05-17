	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 11, 0	sdk_version 12, 1
	.intel_syntax noprefix
	.globl	_test                           ## -- Begin function test
	.p2align	4, 0x90
_test:                                  ## @test
	.cfi_startproc
## %bb.0:
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset rbp, -16
	mov	rbp, rsp
	.cfi_def_cfa_register rbp
	mov	eax, dword ptr [rbp + 32]
	mov	eax, dword ptr [rbp + 24]
	mov	eax, dword ptr [rbp + 16]
	mov	dword ptr [rbp - 4], edi
	mov	dword ptr [rbp - 8], esi
	mov	dword ptr [rbp - 12], edx
	mov	dword ptr [rbp - 16], ecx
	mov	dword ptr [rbp - 20], r8d
	mov	dword ptr [rbp - 24], r9d
	mov	dword ptr [rbp + 32], 199
	mov	eax, dword ptr [rbp - 4]
	add	eax, dword ptr [rbp - 8]
	add	eax, dword ptr [rbp - 12]
	add	eax, dword ptr [rbp - 16]
	add	eax, dword ptr [rbp - 20]
	add	eax, dword ptr [rbp - 24]
	add	eax, dword ptr [rbp + 16]
	add	eax, dword ptr [rbp + 24]
	add	eax, dword ptr [rbp + 32]
	pop	rbp
	ret
	.cfi_endproc
                                        ## -- End function
	.globl	_test1                          ## -- Begin function test1
	.p2align	4, 0x90
_test1:                                 ## @test1
	.cfi_startproc
## %bb.0:
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset rbp, -16
	mov	rbp, rsp
	.cfi_def_cfa_register rbp
	lea	rax, [rbp + 16]
	mov	dword ptr [rbp - 4], edi
	mov	dword ptr [rax + 12], 100
	mov	eax, dword ptr [rax + 8]
	pop	rbp
	ret
	.cfi_endproc
                                        ## -- End function
	.globl	_test2                          ## -- Begin function test2
	.p2align	4, 0x90
_test2:                                 ## @test2
	.cfi_startproc
## %bb.0:
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset rbp, -16
	mov	rbp, rsp
	.cfi_def_cfa_register rbp
	xor	eax, eax
	mov	dword ptr [rbp - 8], edi
	pop	rbp
	ret
	.cfi_endproc
                                        ## -- End function
	.globl	_test3                          ## -- Begin function test3
	.p2align	4, 0x90
_test3:                                 ## @test3
	.cfi_startproc
## %bb.0:
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset rbp, -16
	mov	rbp, rsp
	.cfi_def_cfa_register rbp
	mov	cl, dil
	xor	eax, eax
	mov	byte ptr [rbp - 8], cl
	pop	rbp
	ret
	.cfi_endproc
                                        ## -- End function
	.globl	_main                           ## -- Begin function main
	.p2align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## %bb.0:
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset rbp, -16
	mov	rbp, rsp
	.cfi_def_cfa_register rbp
	sub	rsp, 48
	mov	edi, 10
	mov	esi, 11
	mov	edx, 12
	mov	ecx, 13
	mov	r8d, 14
	mov	r9d, 15
	mov	dword ptr [rsp], 16
	mov	dword ptr [rsp + 8], 17
	mov	dword ptr [rsp + 16], 18
	call	_test
	lea	rax, [rbp - 24]
	mov	rcx, qword ptr [rax]
	mov	qword ptr [rsp], rcx
	mov	rcx, qword ptr [rax + 8]
	mov	qword ptr [rsp + 8], rcx
	mov	eax, dword ptr [rax + 16]
	mov	dword ptr [rsp + 16], eax
	mov	edi, 123
	call	_test1
	xor	eax, eax
	add	rsp, 48
	pop	rbp
	ret
	.cfi_endproc
                                        ## -- End function
.subsections_via_symbols
