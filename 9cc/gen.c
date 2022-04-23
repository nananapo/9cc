#include "9cc.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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

// 四則演算
void	gen_calc(Node *node)
{
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

bool	gen_block(Node *node)
{
	int	lend;
	int	lbegin;

	switch (node->kind)
	{
		case ND_BLOCK:
			while(node != NULL)
			{
				if (node->lhs == NULL)
					break;
				gen(node->lhs);
				if (!is_block_node(node->lhs))
					printf("    pop rax\n");
				node = node->rhs;
			}
			return true;
		case ND_IF:
			// if
			gen(node->lhs);
			printf("    pop rax\n");
			printf("    cmp rax, 0\n");

			lend = jumpLabelCount++;

			if (node->els == NULL)
			{
				printf("    je .Lend%d\n", lend);
				gen(node->rhs);
				if (!is_block_node(node->rhs))
					printf("    pop rax\n");
			}
			else
			{
				int lelse = jumpLabelCount++;
				printf("    je .Lelse%d\n", lelse);
				
				// if stmt
				gen(node->rhs);
				if (!is_block_node(node->rhs))
					printf("    pop rax\n");
				printf("    jmp .Lend%d\n", lend);

				// else
				printf(".Lelse%d:\n", lelse);
				gen(node->els);
				if (!is_block_node(node->els))
					printf("    pop rax\n");
			}
			printf(".Lend%d:\n", lend);
			return true;
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
			return true;
		case ND_FOR:
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;
			
			// init
			if (node->for_init != NULL)
			{
				gen(node->for_init);
				printf("    pop rax\n");
			}
			printf(".Lbegin%d:\n", lbegin);
			
			// if
			if(node->for_if != NULL)
			{
				gen(node->for_if);
				printf("    pop rax\n");
				printf("    cmp rax, 0\n");
				printf("    je .Lend%d\n", lend);
			}

			// for-block
			gen(node->lhs);
			if (!is_block_node(node->lhs))
				printf("    pop rax\n");

			// next
			if(node->for_next != NULL)
			{
				gen(node->for_next);
				printf("    pop rax\n");
			}
			printf("    jmp .Lbegin%d\n", lbegin);
			
			//end
			printf(".Lend%d:\n", lend);
			return true;
		default:
			break;
	}
	return false;
}

void	gen_call(Node *node)
{
	Node	*tmp = node->args;
	int		i = 0;
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
	// returnがある想定
	printf("    push rax\n");
	return;
}

bool	gen_filescope(Node *node)
{
	int		i = 0;
	
	if (node->kind == ND_FUNCDEF)
	{
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
		return true;
	}
	return false;
}

bool	gen_formula(Node *node)
{
	int		i = 0;
	
	switch (node->kind)
	{
		case ND_NUM:
			printf("    push %d\n", node->val);
			return true;
		case ND_LVAR:
			gen_lval(node);
			printf("    pop rax\n");
			printf("    mov rax, [rax]\n");
			printf("    push rax\n");
			return true;
		case ND_ASSIGN:
			gen_lval(node->lhs);
			gen(node->rhs);
			
			printf("    pop rdi\n");
			printf("    pop rax\n");
			printf("    mov [rax], rdi\n");
			printf("    push rdi\n");
			return true;
		case ND_RETURN:
			gen(node->lhs);
			printf("    pop rax\n");
			printf("    mov rsp, rbp\n");
			printf("    pop rbp\n");
			printf("    ret\n");
			return true;
		case ND_CALL:
			gen_call(node);
			return true;
		default:
			break;
	}
	return false;
}

void	gen(Node *node)
{

	if (gen_filescope(node)) return;
	if (gen_block(node)) return;
	if (gen_formula(node)) return;
	gen_calc(node);
}
