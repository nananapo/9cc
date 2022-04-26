#include "9cc.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

int	jumpLabelCount = 0;

#define ARG_REG_COUNT 6
char *arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

int	push_count = 0;

int	align_to(int n, int align)
{
	return (n + align - 1) / align * align;
}

void	push()
{
	push_count++;
	printf("    %s rax\n", ASM_PUSH);
}

void	pushi(int data)
{
	push_count++;
	printf("    %s %d\n", ASM_PUSH, data);
}

void	pop(char *reg)
{
	push_count--;
	printf("    pop %s\n", reg);
}

void	comment(char *c)
{
	printf("# %s\n", c);
}

int	type_size(Type *type);

// 変数のアドレスをpushする
void	gen_lval(Node *node)
{
	comment("lval");

	if (node->kind != ND_LVAR)
		error("代入の左辺値が変数ではありません");
	printf("    %s rax, rbp\n", ASM_MOV);
	printf("    sub rax, %d\n", node->offset);
}

// 四則演算
void	gen_calc(Node *node)
{
	switch (node->kind)
	{
		case ND_ADD:
		case ND_SUB:
			if (node->type->ty == PTR)
			{
				// 左辺をポインタ型にする
				if (node->rhs->type->ty == PTR)
				{
					Node *tmp = node->lhs;
					node->lhs = node->rhs;
					node->rhs = tmp;
				}
				// sizeを求める
				int	size;
				if (node->lhs->type->ptr_to->ty == INT)
				{
					size = 4;
				}
				else
				{
					size = 8;
				}

				// 右辺を掛け算に置き換える
				Node *size_node = new_node(ND_NUM, NULL, NULL);
				size_node->val = size;
				size_node->type = new_primitive_type(INT);
				node->rhs = new_node(ND_MUL, node->rhs, size_node);
				size_node->type = new_primitive_type(INT);
			}
			break;
		default:
			break;
	}

	// TODO ここも怪しい
	gen(node->rhs);
	push();
	gen(node->lhs);

	pop("rdi"); // rhs = rdi

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
				node = node->rhs;
			}
			return true;
		case ND_IF:
			// if
			gen(node->lhs);
			printf("    cmp rax, 0\n");

			lend = jumpLabelCount++;

			if (node->els == NULL)
			{
				printf("    je .Lend%d\n", lend);
				gen(node->rhs);
			}
			else
			{
				int lelse = jumpLabelCount++;
				printf("    je .Lelse%d\n", lelse);
				
				// if stmt
				gen(node->rhs);
				printf("    jmp .Lend%d\n", lend);

				// else
				printf(".Lelse%d:\n", lelse);
				gen(node->els);
			}
			printf(".Lend%d:\n", lend);
			return true;
		case ND_WHILE:
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;
			
			printf(".Lbegin%d:\n", lbegin);
			
			// if
			gen(node->lhs);
			printf("    cmp rax, 0\n");
			printf("    je .Lend%d\n", lend);
			
			// while block
			gen(node->rhs);
			
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
				gen(node->for_init);

			printf(".Lbegin%d:\n", lbegin);
			
			// if
			if(node->for_if != NULL)
			{
				gen(node->for_if);
				printf("    cmp rax, 0\n");
				printf("    je .Lend%d\n", lend);
			}

			// for-block
			gen(node->lhs);

			// next
			if(node->for_next != NULL)
				gen(node->for_next);

			printf("    jmp .Lbegin%d\n", lbegin);
			
			//end
			printf(".Lend%d:\n", lend);
			return true;
		case ND_DEFVAR:
			return true;
		default:
			break;
	}
	return false;
}

void	gen_call(Node *node)
{
	Node	*tmp = node->args;
	int		i;
	
	i = 0;
	while (tmp != NULL && i < ARG_REG_COUNT)
	{
		gen(tmp);
		printf("    %s %s, rax\n", ASM_MOV, arg_regs[node->argdef_count - i - 1]);
		tmp = tmp->next;
		i++;
	}

	comment("call func");
	printf("    call _%s\n", strndup(node->fname, node->flen));
	return;
}

void	init_stack_size(Node *node)
{
	node->stack_size = 0;
	for  (LVar *var = node->locals;var;var = var->next)
	{
		node->stack_size += type_size(var->type);
	}
}

static void	epilogue()
{
	// epilogue
	printf("    %s rsp, rbp\n", ASM_MOV);
	pop("rbp");
	printf("    ret\n");
}

bool	gen_filescope(Node *node)
{
	int		i = 0;
	
	if (node->kind == ND_FUNCDEF)
	{
		printf("_%s:\n", strndup(node->fname, node->flen));
	
		push_count = 0; // pushを初期化

		// prologue
		printf("    %s rax, rbp\n", ASM_MOV);
		push();
		printf("    %s rbp, rsp\n", ASM_MOV);

		init_stack_size(node);
		if (node->stack_size != 0)
		{
			printf("    sub rsp, %d\n", align_to(node->stack_size, 16));

			i = 0;
			printf("    mov rax, rbp\n");
			while (i < node->argdef_count && i < ARG_REG_COUNT)
			{
				printf("    sub rax, 8\n");
				printf("    mov [rax], %s\n", arg_regs[i++]);
			}
		}

		gen(node->lhs);
		epilogue();
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
			printf("    mov rax, %d\n", node->val);
			return true;
		case ND_LVAR:
			gen_lval(node);
			printf("    %s rax, [rax]\n", ASM_MOV);
			return true;
		case ND_ASSIGN:
			if (node->lhs->kind == ND_LVAR)
				gen_lval(node->lhs);
			else if (node->lhs->kind == ND_DEREF)
				gen(node->lhs->lhs);
			else
				error("代入の左辺値が識別子かアドレスではありません");

			push();
			gen(node->rhs);
			pop("rdi");

			printf("    %s [rdi], rax\n", ASM_MOV);
			return true;
		case ND_RETURN:
			gen(node->lhs);
			epilogue();
			return true;
		case ND_CALL:
			gen_call(node);
			return true;
		case ND_ADDR:
			gen_lval(node->lhs);
			return true;
		case ND_DEREF:
			gen(node->lhs);
			printf("    %s rax,[rax]\n", ASM_MOV);
			return true;
		default:
			break;
	}
	return false;
}

void	gen(Node *node)
{
	if (node->kind == ND_PROTOTYPE)
		return;
	if (gen_filescope(node)) return;
	if (gen_block(node)) return;
	if (gen_formula(node)) return;
	gen_calc(node);
}
