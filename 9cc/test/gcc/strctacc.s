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
	mov	dword ptr [rbp - 4], 0
	movsx	eax, byte ptr [rbp - 8]
	movsx	ecx, byte ptr [rbp - 7]
	add	eax, ecx
	pop	rbp
	ret
	.cfi_endproc
                                        ## -- End function
.subsections_via_symbols
