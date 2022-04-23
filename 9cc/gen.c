#include "9cc.h"
#include <stdio.h>
#include <string.h>

int	jumpLabelCount = 0;

#define ARG_REG_COUNT 6
char *arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

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
	int		lend;
	int		lbegin;
	Node	*tmp = node;
	int		i = 0;

	switch (node->kind)
	{
		case ND_FUNCDEF:
			printf("_%s:\n", strndup(node->fname, node->flen));
		
			// prologue
			printf("    push rbp\n");
			printf("    mov rbp, rsp\n");
			i = 0;
			while (i < node->argdef_count && i < ARG_REG_COUNT)
			{
				printf("    push %s\n", arg_regs[i]);
				i++;
			}
			printf("    sub rsp, %d\n", (node->locals_len - node->argdef_count) * 8);
			
			gen(node->lhs);
			if (!is_block_node(node->lhs))
				printf("    pop rax\n");

			//epi
			printf("    mov rsp, rbp\n");
			printf("    pop rbp\n");
			printf("    ret\n");
			return;
		case ND_NUM:
			printf("    push %d\n", node->val);
			return;
		case ND_BLOCK:
			while(node != NULL)
			{
				if (node->lhs == NULL)
					return;
				gen(node->lhs);
				if (!is_block_node(node->lhs))
					printf("    pop rax\n");
				node = node->rhs;
			}
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
			// if
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
				if (!is_block_node(node->lhs))
					printf("    pop rax\n");

				printf("    jmp .Lend%d\n", lend);

				printf(".Lelse%d:\n", lelse);
				gen(node->rhs);
				if (!is_block_node(node->rhs))
					printf("    pop rax\n");
			}
			else
			{
				printf("    je .Lend%d\n", lend);
				gen(node->rhs);
				if (!is_block_node(node->rhs))
					printf("    pop rax\n");
			}
			printf(".Lend%d:\n", lend);
			return;
		case ND_WHILE:
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;
			
			printf(".Lbegin%d:\n", lbegin);
			
			// if
			gen(node->lhs);
			printf("    pop rax\n");
			printf("    cmp rax, 0\n");
			printf("    je .Lend%d\n", lend);
			
			// while block
			gen(node->rhs);
			if (!is_block_node(node->rhs))
				printf("    pop rax\n");
			
			// next
			printf("    jmp .Lbegin%d\n", lbegin);
			
			// end
			printf(".Lend%d:\n", lend);
			return;
		case ND_FOR:
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;
			
			// init
			if (node->lhs != NULL)
			{
				gen(node->lhs);
				printf("    pop rax\n");
			}
			printf(".Lbegin%d:\n", lbegin);
			
			// if
			if(node->rhs->lhs != NULL)
			{
				gen(node->rhs->lhs);
				printf("    pop rax\n");
				printf("    cmp rax, 0\n");
				printf("    je .Lend%d\n", lend);
			}

			// for-block
			gen(node->rhs->rhs->rhs);
			if (!is_block_node(node->rhs->rhs->rhs))
				printf("    pop rax\n");

			// next
			if(node->rhs->rhs->lhs != NULL)
			{
				gen(node->rhs->rhs->lhs);
				printf("    pop rax\n");
			}
			printf("    jmp .Lbegin%d\n", lbegin);
			
			//end
			printf(".Lend%d:\n", lend);
			return;
		case ND_CALL:
			tmp = node->args;
			i = 0;
			while (tmp != NULL && i < ARG_REG_COUNT)
			{
				if (tmp->lhs != NULL)
					gen(tmp->lhs);
				tmp = tmp->rhs;
				i++;
			}

			tmp = node->args;
			i = 0;
			while (tmp != NULL && i < ARG_REG_COUNT)
			{
				if (tmp->lhs != NULL)
				{
					printf("    pop rax\n");
					printf("    mov %s, rax\n", arg_regs[i]);
				}
				tmp = tmp->rhs;
				i++;
			}

			printf("    call _%s\n", strndup(node->fname, node->flen));
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
