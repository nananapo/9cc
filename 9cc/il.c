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



static void	translate_condtional_and(t_node *node);
static void	translate_condtional_or(t_node *node);
static void	translate_call(t_node *node);
static void	translate_return(t_node *node);
static void	translate_block(t_node *node);
static void	translate_if(t_node *node);
static void	translate_while(t_node *node);
static void	translate_dowhile(t_node *node);
static void	translate_for(t_node *node);
static void	translate_switch(t_node *node);
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

static t_il	*append_il_pop(t_type *type)
{
	t_il	*code;
	code		= append_il(IL_POP);
	code->type	= type;
	return (code);
}

static void	translate_condtional_and(t_node *node)
{
	int		lend;
	t_il	*code;

	lend = ++jumpLabelCount;

	// 左辺を評価
	translate_node(node->lhs);
	append_il_pushnum(0);

	code		= append_il(IL_NEQUAL);
	code->type	= node->lhs->type; // TODO int?

	// 結果をもう一回push
	code		= append_il(IL_PUSH_AGAIN);
	code->type	= node->type;

	// 0ならジャンプ
	code			= append_il(IL_JUMP_EQUAL);
	code->label_str	= get_label_str(lend);

	// 右辺を評価
	translate_node(node->rhs);
	append_il_pushnum(0);

	code		= append_il(IL_NEQUAL);
	code->type	= node->rhs->type; // TODO int?

	// 足し算 1 + 1
	code		= append_il(IL_ADD);
	code->type	= node->type;

	// ラベル
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);

	// 2と比較
	append_il_pushnum(2);
	code		= append_il(IL_EQUAL);
	code->type	= node->type; // TODO int?
}

static void	translate_condtional_or(t_node *node)
{
	int		lend;
	t_il	*code;

	lend = ++jumpLabelCount;

	// 左辺を評価
	translate_node(node->lhs);
	append_il_pushnum(0);

	code		= append_il(IL_NEQUAL);
	code->type	= node->lhs->type; // TODO int?

	// 結果をもう一回push
	code		= append_il(IL_PUSH_AGAIN);
	code->type	= node->type;

	// 0以外ならジャンプ
	code			= append_il(IL_JUMP_NEQUAL);
	code->label_str	= get_label_str(lend);

	// 結果は使わないのでpopする
	append_il_pop(node->type);

	// 右辺を評価
	translate_node(node->rhs);
	append_il_pushnum(0);

	code		= append_il(IL_NEQUAL);
	code->type	= node->rhs->type; // TODO int?

	// ラベル
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);
}

static void	translate_call(t_node *node)
{
	int		i;
	t_il	*code;

	code					= append_il(IL_CALL_START);
	code->funccall_caller	= node->funccall_caller;
	code->funccall_callee	= node->funcdef;
	code->funccall_argcount	= node->funccall_argcount;
	code->funccall_argdefs	= node->funccall_argdefs;
	code->funccall_save_pos	= node->call_mem_stack;

	for (i = 0;	i < node->funccall_argcount; i++)
	{
		translate_node(node->funccall_args[i]);
		code						= append_il(IL_CALL_ADD_ARG);
		code->funccall_caller		= node->funccall_caller;
		code->funccall_callee		= node->funcdef;
		code->funccall_arg_index	= i;
		code->funccall_arg_def		= node->funccall_argdefs[i];
		code->type					= type_array_to_ptr(node->funccall_args[i]->type); //TODO これanalyzeにやらせたい
	}

	code					= append_il(IL_CALL_EXEC);
	code->funccall_caller	= node->funccall_caller;
	code->funccall_callee	= node->funcdef;
	code->funccall_argcount	= node->funccall_argcount;
	code->funccall_argdefs	= node->funccall_argdefs;
	code->funccall_save_pos	= node->call_mem_stack;
}

static void	translate_return(t_node *node)
{
	t_il	*code;

	if (node->lhs != NULL)
		translate_node(node->lhs);
	code			= append_il(IL_JUMP);
	code->label_str	= get_function_epi_label(node->funcdef->name, node->funcdef->name_len);
}

static void	translate_block(t_node *node)
{
	t_node	*last;
	t_il	*code;

	last = NULL;
	for (;node != NULL && node->lhs != NULL; node = node->rhs)
	{
		translate_node(node->lhs);
		append_il_pop(node->lhs->type);
		last = node->lhs;
	}

	// 最後の結果をpushする
	if (last != NULL)
	{
		code		= append_il(IL_PUSH_AGAIN);
		code->type	= last->type;
	}
}

static void	translate_if(t_node *node)
{
	t_il	*code;
	int		lend;
	int		lelse;

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
	int		lbegin;
	int		lend;
	t_il	*code;

	lbegin	= ++jumpLabelCount;
	lend	= ++jumpLabelCount;

	// ラベルを割当
	node->block_sbdata->startLabel	= lbegin;
	node->block_sbdata->endLabel	= lend;

	// continue先
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lbegin);

	// while (lhs)
	translate_node(node->lhs);
	append_il_pushnum(0);

	code		= append_il(IL_EQUAL);
	code->type	= node->lhs->type;

	code			= append_il(IL_JUMP_EQUAL);
	code->label_str	= get_label_str(lend);

	// while () rhs
	if (node->rhs != NULL)
	{
		translate_node(node->rhs);
		append_il_pop(node->rhs->type);
	}

	// next
	code			= append_il(IL_JUMP);
	code->label_str	= get_label_str(lbegin);
	
	// end
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);

	append_il_pushnum(0);
}

static void	translate_dowhile(t_node *node)
{
	int		lbegin;
	int		lbegin2;
	int		lend;
	t_il	*code;

	lbegin	= ++jumpLabelCount;
	lbegin2	= ++jumpLabelCount;
	lend	= ++jumpLabelCount;

	// ラベルを割当
	node->block_sbdata->startLabel	= lbegin2;
	node->block_sbdata->endLabel	= lend;

	// start
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lbegin);

	// while block
	if (node->lhs != NULL)
	{
		translate_node(node->lhs);
		append_il_pop(node->lhs->type);
	}

	// continue
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lbegin2);

	// if
	translate_node(node->rhs);
	append_il_pushnum(0);

	code		= append_il(IL_NEQUAL);
	code->type	= node->rhs->type;

	code			= append_il(IL_JUMP_NEQUAL);
	code->label_str	= get_label_str(lbegin);

	// end
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);

	append_il_pushnum(0);
}

static void	translate_for(t_node *node)
{
	int		lbegin;
	int		lbegin2;
	int		lend;
	t_il	*code;

	lbegin	= ++jumpLabelCount;
	lbegin2	= ++jumpLabelCount;
	lend	= ++jumpLabelCount;

	// ラベルを割当
	node->block_sbdata->startLabel	= lbegin2;
	node->block_sbdata->endLabel	= lend;

	// for (0;)
	if (node->for_expr[0] != NULL)
	{
		translate_node(node->for_expr[0]);
		append_il_pop(node->for_expr[0]->type);
	}

	// continue
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lbegin);
	
	// for (;1;)
	if(node->for_expr[1] != NULL)
	{
		translate_node(node->for_expr[1]);
		append_il_pushnum(0);

		code		= append_il(IL_EQUAL);
		code->type	= node->for_expr[1]->type; // TODO これ、構造体かどうかとかのチェックしてます?

		code			= append_il(IL_JUMP_EQUAL);
		code->label_str = get_label_str(lend);
	}

	// for() lhs
	if (node->lhs != NULL)
	{
		translate_node(node->lhs);
		append_il_pop(node->lhs->type);
	}

	// continue
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lbegin2);

	// for (;;2)
	if(node->for_expr[2] != NULL)
	{
		translate_node(node->for_expr[2]);
		append_il_pop(node->for_expr[2]->type);
	}

	code			= append_il(IL_JUMP);
	code->label_str	= get_label_str(lbegin);
	
	//end
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);

	append_il_pushnum(0);
}

static void	translate_switch(t_node *node)
{
	int				lbegin;
	int				lend;
	t_il			*code;
	t_switchcase	*cases;

	lbegin	= ++jumpLabelCount;
	lend	= ++jumpLabelCount;

	// ラベルを割当
	node->block_sbdata->startLabel		= -1; // TODO これ必要？
	node->block_sbdata->endLabel		= lend;
	node->block_sbdata->defaultLabel	= lbegin;

	// ケースのラベルに数字を振る
	for (cases = node->block_sbdata->cases; cases != NULL; cases = cases->next)
		cases->label = ++jumpLabelCount;

	// switch (lhs)
	translate_node(node->lhs);

	// 結果を保存
	code			= append_il(IL_ASSIGN);
	code->type		= node->lhs->type; // TODO これint?
	code->var_local	= node->switch_save;

	// if
	for (cases = node->block_sbdata->cases; cases != NULL; cases = cases->next)
	{
		code			= append_il(IL_VAR_LOCAL);
		code->var_local	= node->switch_save;

		append_il_pushnum(cases->value);

		code		= append_il(IL_EQUAL);
		code->type	= node->lhs->type;

		code			= append_il(IL_JUMP_EQUAL);
		code->label_str	= get_label_str(cases->label);
	}

	// defaultかendに飛ばす
	code			= append_il(IL_JUMP);
	if (node->switch_has_default)
		code->label_str	= get_label_str(lbegin);
	else
		code->label_str	= get_label_str(lend);

	translate_node(node->rhs);

	//end
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);
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
			translate_block(node);
			return ;
		case ND_NUM:
			append_il_pushnum(node->val);
			return ;

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
		case ND_BITWISE_AND:
		case ND_BITWISE_OR:
		case ND_BITWISE_XOR:
		case ND_SHIFT_LEFT:
		case ND_SHIFT_RIGHT:
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
			else if (node->kind == ND_LESSEQ)
				code = append_il(IL_LESSEQ);
			else if (node->kind == ND_BITWISE_AND)
				code = append_il(IL_BITWISE_AND);
			else if (node->kind == ND_BITWISE_OR)
				code = append_il(IL_BITWISE_OR);
			else if (node->kind == ND_BITWISE_XOR)
				code = append_il(IL_BITWISE_XOR);
			else if (node->kind == ND_SHIFT_LEFT)
				code = append_il(IL_SHIFT_LEFT);
			else //if (node->kind == ND_SHIFT_RIHGT)
				code = append_il(IL_SHIFT_RIGHT);

			code->type = node->type;
			return ;
		}
		case ND_BITWISE_NOT:
		{
			translate_node(node->lhs);
			code		= append_il(IL_BITWISE_NOT);
			code->type	= node->lhs->type;
			return ;
		}
		case ND_COMP_ADD:
		case ND_COMP_SUB:
		case ND_COMP_MUL:
		case ND_COMP_DIV:
		case ND_COMP_MOD:
		{
			// 左辺を二回push -> load -> 右辺をpush -> swap -> op -> assign
			translate_node(node->lhs);
			code		= append_il(IL_PUSH_AGAIN);
			code->type	= node->lhs->type;

			code		= append_il(IL_LOAD);
			code->type	= node->lhs->type->ptr_to;

			translate_node(node->rhs);

			code			= append_il(IL_STACK_SWAP);
			code->stack_up	= node->rhs->type;
			code->stack_down= node->lhs->type->ptr_to;

			// op
			if (node->kind == ND_COMP_ADD)
				code	= append_il(IL_ADD);
			else if (node->kind == ND_COMP_SUB)
				code	= append_il(IL_SUB);
			else if (node->kind == ND_COMP_MUL)
				code	= append_il(IL_MUL);
			else if (node->kind == ND_COMP_DIV)
				code	= append_il(IL_DIV);
			else if (node->kind == ND_COMP_MOD)
				code	= append_il(IL_MOD);
			code->type	= node->type;

			code		= append_il(IL_ASSIGN);
			code->type	= node->type;
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
			code			= append_il(IL_VAR_LOCAL);
			code->var_local	= node->lvar;
			return ;
		}
		case ND_VAR_LOCAL_ADDR:
		{
			code			= append_il(IL_VAR_LOCAL_ADDR);
			code->var_local	= node->lvar;
			return ;
		}
		case ND_VAR_GLOBAL:
		{
			code			= append_il(IL_VAR_GLOBAL);
			code->var_global= node->var_global;
			return ;
		}
		case ND_VAR_GLOBAL_ADDR:
		{
			code			= append_il(IL_VAR_GLOBAL_ADDR);
			code->var_global= node->var_global;
			return ;
		}
		case ND_COND_AND:
			translate_condtional_and(node);
			return ;
		case ND_COND_OR:
			translate_condtional_or(node);
			return ;
		case ND_CAST:
		{
			translate_node(node->lhs);
			code			= append_il(IL_CAST);
			code->cast_from	= node->lhs->type;
			code->cast_to	= node->type;
			return ;
		}
		case ND_DEREF:
		{
			translate_node(node->lhs);
			code		= append_il(IL_LOAD);
			code->type	= node->type;
			return ;
		}
		case ND_DEREF_ADDR:
		{
			translate_node(node->lhs);
			return ;
		}
		case ND_MEMBER_VALUE:
		{
			translate_node(node->lhs);
			code		= append_il(IL_MEMBER);
			code->member= node->elem;
			return ;
		}
		case ND_MEMBER_VALUE_ADDR:
		{
			translate_node(node->lhs);
			code		= append_il(IL_MEMBER_ADDR);
			code->member= node->elem;
			return ;
		}
		case ND_MEMBER_PTR_VALUE:
		{
			translate_node(node->lhs);
			code		= append_il(IL_MEMBER_PTR);
			code->member= node->elem;
			return ;
		}
		case ND_MEMBER_PTR_VALUE_ADDR:
		{
			translate_node(node->lhs);
			code		= append_il(IL_MEMBER_PTR_ADDR);
			code->member= node->elem;
			return ;
		}
		case ND_CALL_MACRO_VA_START:
		{
			translate_node(node->funccall_args[0]);
			code					= append_il(IL_MACRO_VASTART);
			code->funccall_caller	= node->funccall_caller;
			append_il_pushnum(0);
			return ;
		}
		case ND_STR_LITERAL: // 文字列リテラルのアドレスは?
		{
			code			= append_il(IL_STR_LIT);
			code->def_str	= node->def_str;
			return ;
		}
		case ND_CALL:
			translate_call(node);
			return ;
		case ND_RETURN:
			translate_return(node);
			return ;
		case ND_CONTINUE:
		{
			code			= append_il(IL_JUMP);
			code->label_str	= get_label_str(node->block_sbdata->startLabel);
			append_il_pushnum(0);
			return;
		}
		case ND_BREAK:
		{
			code			= append_il(IL_JUMP);
			code->label_str	= get_label_str(node->block_sbdata->endLabel);
			append_il_pushnum(0);
			return;
		}
		case ND_CASE:
		{
			code			= append_il(IL_LABEL);
			code->label_str	= get_label_str(node->case_label->label);
			append_il_pushnum(0);
			return;
		}
		case ND_DEFAULT:
		{
			code			= append_il(IL_LABEL);
			code->label_str	= get_label_str(node->block_sbdata->defaultLabel);
			append_il_pushnum(0);
			return;
		}
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
			append_il_pushnum(0);
			node->type = new_primitive_type(TY_INT);
			return ;
		case ND_SIZEOF:
			fprintf(stderr, "sizeof not allowed\n"); // TODO genまで持っていく
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

	code				= append_il(IL_FUNC_PROLOGUE);
	code->deffunc_def	= func;

	for (lvar = func->locals; lvar != NULL; lvar = lvar->next)
	{
		// TODO register
		code = append_il(IL_DEF_VAR_LOCAL);
		code->var_local = lvar;
	}
	append_il(IL_DEF_VAR_END);

	translate_node(func->stmt);

	code			= append_il(IL_LABEL);
	code->label_str	= get_function_epi_label(func->name, func->name_len);

	code		= append_il(IL_FUNC_EPILOGUE);
	code->type	= func->type_return;
	code->deffunc_def	= func;
}

void	translate_il(void)
{
	int	i;

	for (i = 0; g_func_defs[i] != NULL; i++)
		translate_func(g_func_defs[i]);
}
