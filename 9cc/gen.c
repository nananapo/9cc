#include "9cc.h"
#include <stdio.h>

// 変数のアドレスをpushする
void	gen_lval(Node *node)
{
	if (node->kind != ND_LVAR)
		error("代入の左辺値が変数ではありません");
	printf("    mov rax, rbp\n");
	printf("    sub rax, %d\n", node->offset);
	printf("    push rax\n");
}

void	gen(Node *node)
{
	switch (node->kind)
	{
		case ND_NUM:
			printf("    push %d\n", node->val);
			return;
		case ND_LVAR:
			gen_lval(node);
			printf("    pop rax\n");
			printf("    mov rax, [rax]\n");
			printf("    push rax\n");
			return;
		case ND_ASSIGN:
			gen_lval(node->lhs);
			gen(node->rhs);
			
			printf("    pop rdi\n");
			printf("    pop rax\n");
			printf("    mov [rax], rdi\n");
			printf("    push rdi\n");
			return;
		case ND_RETURN:
			gen(node->lhs);
			printf("    pop rax\n");
			printf("    mov rsp, rbp\n");
			printf("    pop rbp\n");
			printf("    ret\n");
			return;
		default:
			break;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("    pop rdi\n");
	printf("    pop rax\n");

	switch (node->kind)
	{
		case ND_ADD:
			printf("    add rax, rdi\n");
			break;
		case ND_SUB:
			printf("    sub rax, rdi\n");
			break;
		case ND_MUL:
			printf("    imul rax, rdi\n");
			break;
		case ND_DIV:
			printf("    cqo\n");
			printf("    idiv rdi\n");
			break;
		case ND_EQUAL:
			printf("    cmp rdi, rax\n");
			printf("    sete al\n");
			printf("    movzx rax, al\n");
			break;
		case ND_NEQUAL:
			printf("    cmp rdi, rax\n");
			printf("    setne al\n");
			printf("    movzx rax, al\n");
			break;
		case ND_LESS:
			printf("    cmp rax, rdi\n");
			printf("    setl al\n");
			printf("    movzx rax, al\n");
			break;
		case ND_LESSEQ:
			printf("    cmp rax, rdi\n");
			printf("    setle al\n");
			printf("    movzx rax, al\n");
			break;
		default:
			fprintf(stderr, "不明なノード\n");
			break;
	}
	printf("    push rax\n");
}
