	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 11, 0	sdk_version 12, 1
	.intel_syntax noprefix
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
	xor	eax, eax
	mov	dword ptr [rbp - 24], 123456
	mov	dword ptr [rbp - 20], 987655
	mov	byte ptr [rbp - 16], 107
	mov	byte ptr [rbp - 15], 106
	mov	dword ptr [rbp - 12], 999
	mov	qword ptr [rbp - 8], 0
	pop	rbp
	ret
	.cfi_endproc
                                        ## -- End function
.subsections_via_symbols
