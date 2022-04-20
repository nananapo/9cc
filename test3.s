.intel_syntax noprefix
.globl plus, _main

plus:
	add rsi, rdi
	mov rax, rsi
	ret

_main:
	mov rdi, 3
	mov rsi, 4
	call plus
	ret
