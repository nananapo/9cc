.intel_syntax noprefix
.global _fib, _main
_fib:
    push rbp
    mov rbp, rsp
    push rdi
    sub rsp, 0
    mov rax, rbp
    sub rax, 0
    push rax
    pop rax
    mov rax, [rax]
    push rax
    push 0
    pop rdi
    pop rax
    cmp rdi, rax
    sete al
    movzx rax, al
    push rax
    pop rax
    cmp rax, 0
    je .Lend0
    push 1
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    pop rax
.Lend0:
    mov rax, rbp
    sub rax, 0
    push rax
    pop rax
    mov rax, [rax]
    push rax
    push 1
    pop rdi
    pop rax
    cmp rdi, rax
    sete al
    movzx rax, al
    push rax
    pop rax
    cmp rax, 0
    je .Lend1
    push 1
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    pop rax
.Lend1:
    mov rax, rbp
    sub rax, 0
    push rax
    pop rax
    mov rax, [rax]
    push rax
    push 2
    pop rdi
    pop rax
    sub rax, rdi
    push rax
    pop rax
    mov rdi, rax
    call _fib
    mov rax, rbp
    sub rax, 0
    push rax
    pop rax
    mov rax, [rax]
    push rax
    push 1
    pop rdi
    pop rax
    sub rax, rdi
    push rax
    pop rax
    mov rdi, rax
    call _fib
    pop rdi
    pop rax
    add rax, rdi
    push rax
    pop rax
    mov rsp, rbp
    pop rbp
    ret
    pop rax
    mov rsp, rbp
    pop rbp
    ret
_main:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov rax, rbp
    sub rax, 0
    push rax
    push 0
    pop rdi
    pop rax
    mov [rax], rdi
    push rdi
    pop rax
.Lbegin2:
    mov rax, rbp
    sub rax, 0
    push rax
    pop rax
    mov rax, [rax]
    push rax
    push 11
    pop rdi
    pop rax
    cmp rax, rdi
    setl al
    movzx rax, al
    push rax
    pop rax
    cmp rax, 0
    je .Lend3
    mov rax, rbp
    sub rax, 8
    push rax
    mov rax, rbp
    sub rax, 0
    push rax
    pop rax
    mov rax, [rax]
    push rax
    pop rax
    mov rdi, rax
    call _fib
    pop rdi
    pop rax
    mov [rax], rdi
    push rdi
    pop rax
    mov rax, rbp
    sub rax, 8
    push rax
    pop rax
    mov rax, [rax]
    push rax
    pop rax
    mov rdi, rax
    call _pint
    pop rax
    push 1
    pop rax
    mov rdi, rax
    call _pspace
    pop rax
    mov rax, rbp
    sub rax, 0
    push rax
    mov rax, rbp
    sub rax, 0
    push rax
    pop rax
    mov rax, [rax]
    push rax
    push 1
    pop rdi
    pop rax
    add rax, rdi
    push rax
    pop rdi
    pop rax
    mov [rax], rdi
    push rdi
    pop rax
    jmp .Lbegin2
.Lend3:
    mov rsp, rbp
    pop rbp
    ret
