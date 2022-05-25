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
	sub	rsp, 32
	mov	eax, dword ptr [rbp + 40]
	mov	eax, dword ptr [rbp + 32]
	mov	eax, dword ptr [rbp + 24]
	mov	eax, dword ptr [rbp + 16]
	mov	dword ptr [rbp - 4], edi
	mov	dword ptr [rbp - 8], esi
	mov	dword ptr [rbp - 12], edx
	mov	dword ptr [rbp - 16], ecx
	mov	dword ptr [rbp - 20], r8d
	mov	dword ptr [rbp - 24], r9d
	mov	edi, dword ptr [rbp - 4]
	call	_dint
	mov	edi, dword ptr [rbp - 8]
	call	_dint
	mov	edi, dword ptr [rbp - 12]
	call	_dint
	mov	edi, dword ptr [rbp - 16]
	call	_dint
	mov	edi, dword ptr [rbp - 20]
	call	_dint
	mov	edi, dword ptr [rbp - 24]
	call	_dint
	mov	edi, dword ptr [rbp + 16]
	call	_dint
	mov	edi, dword ptr [rbp + 24]
	call	_dint
	mov	edi, dword ptr [rbp + 32]
	call	_dint
	mov	edi, dword ptr [rbp + 40]
	call	_dint
	mov	eax, dword ptr [rbp + 32]
	add	rsp, 32
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
	sub	rsp, 32
	mov	edi, 1
	mov	esi, 3
	mov	edx, 5
	mov	ecx, 7
	mov	r8d, 9
	mov	r9d, 16
	mov	dword ptr [rsp], 22
	mov	dword ptr [rsp + 8], 46
	mov	dword ptr [rsp + 16], 12
	mov	dword ptr [rsp + 24], 99
	call	_test
	mov	edi, eax
	call	_dint
	xor	eax, eax
	add	rsp, 32
	pop	rbp
	ret
	.cfi_endproc
                                        ## -- End function
.subsections_via_symbols
