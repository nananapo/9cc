#include "9cc.h"
#include "il.h"
#include "charutil.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// main
extern t_deffunc		*g_func_defs[1000];
extern t_il				*g_il;

static t_il	*g_il_last;
static int	jumpLabelCount;



static void	translate_return(t_node *node);
static void	translate_if(t_node *node);
static void	translate_while(t_node *node)
static void	translate_dowhile(t_node *node)
static void	translate_for(t_node *node)
static void	translate_switch(t_node *node)
static void	translate_node(t_node *node);
static void	translate_func(t_deffunc *func);
void		translate_il(void);


static char	*get_function_epi_label(char *name, int len)
{
	return (my_strcat("Lepi_", strndup(name, len)));
}

static char	*get_label_str(int i)
{
	char	buf[100];
	sprintf(buf, "%d", i);
	return (my_strcat("L", buf));
}

static t_il	*append_il(t_ilkind kind)
{
	t_il	*tmp;

	tmp			= calloc(1, sizeof(t_il));
	tmp->kind	= kind;

	if (g_il == NULL)
		g_il = tmp;
	else
		g_il_last->next = tmp;
	g_il_last = tmp;
	return (tmp);
}

static t_il	*append_il_pushnum(int i)
{
	t_il	*code;
	code			= append_il(IL_PUSH_NUM);
	code->number_int= i;
	return (code);
}

static void	translate_return(t_node *node)
{
	t_il	*code;

	if (node->lhs != NULL)
	{
		translate_node(node->lhs);
		code = append_il(IL_POP);
		code->type = node->lhs->type;
	}
	code			= append_il(IL_JUMP);
	code->label_str	= get_function_epi_label(node->funcdef->name, node->funcdef->name_len);
	return ;
}

static void	translate_if(t_node *node)
{
	t_il	*code;
	int		lend;
	int		lelse;

	// assign label
	lend	= ++jumpLabelCount;
	lelse	= ++jumpLabelCount;

	translate_node(node->lhs);
	append_il_pushnum(0);

	code		= append_il(IL_EQUAL);
	code->type	= node->lhs->type;
	if (node->elsif != NULL)
	{
		// falseならelseに飛ぶ
		code			= append_il(IL_JUMP_EQUAL);
		code->label_str	= get_label_str(lelse);

		translate_node(node->rhs);

		code			= append_il(IL_JUMP);
		code->label_str	= get_label_str(lend);

		// else if
		code			= append_il(IL_LABEL);
		code->label_str	= get_label_str(lelse);

		translate_node(node->elsif);
	}
	else if (node->els != NULL)
	{
		// falseならelseに飛ぶ
		code			= append_il(IL_JUMP_EQUAL);
		code->label_str	= get_label_str(lelse);

		translate_node(node->rhs);

		// else if
		code			= append_il(IL_LABEL);
		code->label_str	= get_label_str(lelse);

		translate_node(node->els);
	}
	else
	{
		// else if
		code			= append_il(IL_JUMP_EQUAL);
		code->label_str	= get_label_str(lend);

		translate_node(node->rhs);
	}

	// end label
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);
}

static void	translate_while(t_node *node)
{
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;

			node->block_sbdata->startLabel = lbegin;
			node->block_sbdata->endLabel = lend;

			printf(".Lbegin%d:\n", lbegin); // continue先
			
			// if
			gen(node->lhs);
			mov(RDI, "0");
			cmp(node->lhs->type);
			printf("    je .Lend%d\n", lend);
			
			// while block
			if (node->rhs != NULL)
				gen(node->rhs);
			
			// next
			printf("    jmp .Lbegin%d\n", lbegin);
			
			// end
			printf(".Lend%d:\n", lend); //break先
			return;
}

static void	translate_dowhile(t_node *node)
{
			lbegin = jumpLabelCount++;
			lbegin2 = jumpLabelCount++;
			lend = jumpLabelCount++;

			node->block_sbdata->startLabel = lbegin2;
			node->block_sbdata->endLabel = lend;
			
			printf(".Lbegin%d:\n", lbegin);

			// while block
			if (node->lhs != NULL)
				gen(node->lhs);

			// if
			printf(".Lbegin%d:\n", lbegin2); // continueで飛ぶ先
			gen(node->rhs);
			mov(RDI, "0");
			cmp(node->rhs->type);
			printf("    jne .Lbegin%d\n", lbegin);
			printf(".Lend%d:\n", lend); // break先
			return;
}

static void	translate_for(t_node *node)
{
			lbegin = jumpLabelCount++;
			lbegin2 = jumpLabelCount++;
			lend = jumpLabelCount++;
			
			node->block_sbdata->startLabel = lbegin2;
			node->block_sbdata->endLabel = lend;

			// init
			if (node->for_expr[0] != NULL)
				gen(node->for_expr[0]);

			printf(".Lbegin%d:\n", lbegin);
			
			// if
			if(node->for_expr[1] != NULL)
			{
				gen(node->for_expr[1]);
				mov(RDI, "0");
				cmp(node->for_expr[1]->type);
				printf("    je .Lend%d\n", lend);
			}

			// for-block
			if (node->lhs != NULL)
				gen(node->lhs);

			printf(".Lbegin%d:\n", lbegin2); // continue先
			// next
			if(node->for_expr[2] != NULL)
				gen(node->for_expr[2]);

			printf("    jmp .Lbegin%d\n", lbegin);
			
			//end
			printf(".Lend%d:\n", lend); // break先
			return;
}

static void	translate_switch(t_node *node)
{
			lbegin = jumpLabelCount++;
			lend = jumpLabelCount++;

			// ケースのラベルに数字を振る
			for (cases = node->block_sbdata->cases; cases != NULL; cases = cases->next)
				cases->label = jumpLabelCount++;
	
			// 評価
			gen(node->lhs);
			printf("    mov [rsp - 8], rax\n"); //結果を格納

			// if
			debug("    switch def:%d, end:%d", lbegin, lend);
			for (cases = node->block_sbdata->cases; cases != NULL; cases = cases->next)
			{
				printf("    mov rax, [rsp - 8]\n");
				movi(RDI, cases->value);
				cmp(node->lhs->type);
				printf("    je .Lswitch%d\n", cases->label);
			}
			// defaultかendに飛ばす
			if (node->switch_has_default)
				printf("    jmp .Lswitch%d\n", lbegin);
			else
				printf("    jmp .Lend%d\n", lend);

			debug("    switch in");

			// 文を出力
			node->block_sbdata->startLabel = -1;
			node->block_sbdata->endLabel = lend;
			node->block_sbdata->defaultLabel = lbegin;
			
			gen(node->rhs);

			printf(".Lend%d:\n", lend);
			return ;
}

static void	translate_node(t_node *node)
{
	t_il	*code;

	//fprintf(stderr, "# kind: %d\n", node->kind);
	if (!node->is_analyzed)
		error("t_node is not analyzed");

	switch(node->kind)
	{
		case ND_BLOCK:
		{
			while (node != NULL)
			{
				if (node->lhs == NULL)
					return ;
				translate_node(node->lhs);
				if (node->lhs->kind != ND_NONE)
					append_il(IL_POP);
				node = node->rhs;
			}
			return ;
		}
		case ND_NUM:
		{
			append_il_pushnum(node->val);
			return ;
		}

		// TODO analyzeで両方の型が一致している状態にする
		case ND_ADD:
		case ND_SUB:
		case ND_MUL:
		case ND_DIV:
		case ND_MOD:
		case ND_EQUAL:
		case ND_NEQUAL:
		case ND_LESS:
		case ND_LESSEQ:
		{
			translate_node(node->lhs);
			translate_node(node->rhs);
			
			if (node->kind == ND_ADD)
				code = append_il(IL_ADD);
			else if (node->kind == ND_SUB)
				code = append_il(IL_SUB);
			else if (node->kind == ND_MUL)
				code = append_il(IL_MUL);
			else if (node->kind == ND_DIV)
				code = append_il(IL_DIV);
			else if (node->kind == ND_MOD)
				code = append_il(IL_MOD);
			else if (node->kind == ND_EQUAL)
				code = append_il(IL_EQUAL);
			else if (node->kind == ND_NEQUAL)
				code = append_il(IL_NEQUAL);
			else if (node->kind == ND_LESS)
				code = append_il(IL_LESS);
			else// if (node->kind == ND_LESSEQ)
				code = append_il(IL_LESSEQ);

			code->type = node->type;
			return ;
		}
		case ND_ASSIGN:
		{
			translate_node(node->lhs);
			translate_node(node->rhs);
			code		= append_il(IL_ASSIGN);
			code->type	= node->type;
			return ;
		}
		case ND_VAR_LOCAL:
		{
			code		= append_il(IL_VAR_LOCAL);
			code->lvar	= node->lvar;
			return ;
		}
		case ND_VAR_LOCAL_ADDR:
		{
			code		= append_il(IL_VAR_LOCAL_ADDR);
			code->lvar	= node->lvar;
			return ;
		}
		case ND_RETURN:
			translate_return(node);
			return ;
		case ND_IF:
			translate_if(node);
			return ;
		case ND_WHILE:
			translate_while(node);
			return ;
		case ND_DOWHILE:
			translate_dowhile(node);
			return ;
		case ND_FOR:
			translate_for(node);
			return ;
		case ND_SWITCH:
			translate_switch(node);
			return ;
		case ND_NONE:
			return ;
		case ND_SIZEOF:
			fprintf(stderr, "sizeof not allowed\n", node->kind);
			error("Error");
			return ;
		default:
		{
			fprintf(stderr, "il not implemented\nkind : %d\n", node->kind);
			error("Error");
			return ;
		}
	}
}

static void	translate_func(t_deffunc *func)
{
	t_il	*code;
	t_lvar	*lvar;

	code					= append_il(IL_LABEL);
	code->label_str			= strndup(func->name, func->name_len);
	code->label_is_deffunc	= true;

	append_il(IL_FUNC_PROLOGUE);
	for (lvar = func->locals; lvar != NULL; lvar = lvar->next)
	{
		// TODO register
		code = append_il(IL_DEF_VAR_LOCAL);
		code->lvar = lvar;
	}
	append_il(IL_DEF_VAR_END);

	translate_node(func->stmt);

	code			= append_il(IL_LABEL);
	code->label_str	= get_function_epi_label(func->name, func->name_len);

	append_il(IL_FUNC_EPILOGUE);
}

void	translate_il(void)
{
	int	i;

	for (i = 0; g_func_defs[i] != NULL; i++)
		translate_func(g_func_defs[i]);
}
