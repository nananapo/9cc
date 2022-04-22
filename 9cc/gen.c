#include "9cc.h"
#include <stdio.h>

int	jumpLabelCount = 0;

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
	int	lend;
	int	lbegin;

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
		case ND_IF:
			gen(node->lhs);
			printf("    pop rax\n");
			printf("    cmp rax, 0\n");

			lend = jumpLabelCount++;

			if (node->rhs->kind == ND_ELSE)
			{
				node = node->rhs;
				int lelse = jumpLabelCount++;
				printf("    je .Lelse%d\n", lelse);
				gen(node->lhs);
				printf("    jmp .Lend%d\n", lend);
				printf(".Lelse%d:\n", lelse);
				gen(node->rhs);
				printf("    pop rax\n");
			}
			else
			{
				printf("    je .Lend%d\n", lend);
				gen(node->rhs);
				printf("    pop rax\n");
			}
			printf(".Lend%d:\n", lend);
			return;
		case ND_WHILE:
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;
			printf(".Lbegin%d:\n", lbegin);
			gen(node->lhs);
			printf("    pop rax\n");
			printf("    cmp rax, 0\n");
			printf("    je .Lend%d\n", lend);
			gen(node->rhs);
			printf("    pop rax\n");
			printf("    jmp .Lbegin%d\n", lbegin);
			printf(".Lend%d:\n", lend);
			return;
		case ND_FOR:
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;
			if (node->lhs != NULL)
			{
				gen(node->lhs);
				printf("    pop rax\n");
			}
			printf(".Lbegin%d:\n", lbegin);
			if(node->rhs->lhs != NULL)
			{
				gen(node->rhs->lhs);
				printf("    pop rax\n");
				printf("    cmp rax, 0\n");
				printf("    je .Lend%d\n", lend);
			}
			gen(node->rhs->rhs->rhs);
			printf("    pop rax\n");
			if(node->rhs->rhs->lhs != NULL)
			{
				gen(node->rhs->rhs->lhs);
				printf("    pop rax\n");
			}
			printf("    jmp .Lbegin%d\n", lbegin);
			printf(".Lend%d:\n", lend);
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
			fprintf(stderr, "不明なノード %d\n", node->kind);
			break;
	}
	printf("    push rax\n");
}
