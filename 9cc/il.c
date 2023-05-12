#include "9cc.h"
#include "il.h"
#include "charutil.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern t_ir_func		*g_ir_funcs[1000];
extern t_funcil_pair	*g_func_ils;

static t_il	*g_il;
static t_il	*g_il_last;
static int	jumpLabelCount;
static int	ILID_UNIQUE;

/*
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
*/
static void	translate_func(t_ir_func *func);
void		translate_il(void);

static char	*get_function_epi_label(char *name, int len)
{
	return (my_strcat("Lepi_", my_strndup(name, len)));
}

static char	*get_label_str(int i)
{
	char	buf[1000];
	snprintf(buf, sizeof(buf), "%d", i);
	return (my_strcat("L", buf));
}

char* get_il_name(t_ilkind kind)
{
	switch (kind)
	{
		case IL_LABEL:
			return "LABEL";
		case IL_JUMP:
			return "JUMP";
		case IL_JUMP_TRUE:
			return "JUMP_TRUE";
		case IL_JUMP_FALSE:
			return "JUMP_FALSE";
		case IL_FUNC_PROLOGUE:
			return "FUNC_PROLOGUE";
		case IL_FUNC_EPILOGUE:
			return "EPILOGUE";
		case IL_DEF_VAR_LOCAL:
			return "DEF_VAR_LOCAL";
		case IL_DEF_VAR_LOCAL_ARRAY:
			return "DEF_VAR_LOCAL_ARRAY";
		case IL_DEF_VAR_END:
			return "DEF_VAR_END";
		case IL_PUSH_AGAIN:
			return "PUSH_AGAIN";
		case IL_PUSH_NUM:
			return "PUSH_NUM";
		case IL_PUSH_FLOAT:
			return "PUSH_FLOAT";
		case IL_POP:
			return "POP";
		case IL_ADD:
			return "ADD";
		case IL_SUB:
			return "SUB";
		case IL_MUL:
			return "MUL";
		case IL_DIV:
			return "DIV";
		case IL_MOD:
			return "MOD";
		case IL_EQUAL:
			return "EQUAL";
		case IL_NEQUAL:
			return "NEQUAL";
		case IL_LESS:
			return "LESS";
		case IL_LESSEQ:
			return "LESSQ";
		case IL_BITWISE_AND:
			return "BITWISE_AND";
		case IL_BITWISE_OR:
			return "BITWISE_OR";
		case IL_BITWISE_XOR:
			return "BITWISE_XOR";
		case IL_BITWISE_NOT:
			return "BITWISE_NOT";
		case IL_SHIFT_LEFT:
			return "SHIFT_LEFT";
		case IL_SHIFT_RIGHT:
			return "SHIFT_RIGHT";
		case IL_ASSIGN:
			return "ASSIGN";
		case IL_VAR_LOCAL:
			return "VAR_LOCAL";
		case IL_VAR_LOCAL_ADDR:
			return "VAR_LOCAL_ADDR";
		case IL_VAR_GLOBAL:
			return "VAR_GLOBAL";
		case IL_VAR_GLOBAL_ADDR:
			return "VAR_GLOBAL_ADDR";
		case IL_MEMBER:
			return "MEMBER";
		case IL_MEMBER_ADDR:
			return "MEMBER_ADDER";
		case IL_MEMBER_PTR:
			return "MEMBER_PTR";
		case IL_MEMBER_PTR_ADDR:
			return "MEMBER_PTR_ADDR";
		case IL_STR_LIT:
			return "STR_LIT";
		case IL_CALL_START:
			return "CALL_START";
		case IL_CALL_ADD_ARG:
			return "CALL_ADD_ARG";
		case IL_CALL_EXEC:
			return "CALL_EXEC";
		case IL_MACRO_VASTART:
			return "MACRO_VA_START";
		case IL_CAST:
			return "CAST";
		case IL_LOAD:
			return "LOAD";
	}
}

void	print_il(t_il *code)
{
	debug("#%d %s", code->ilid_unique, get_il_name(code->kind));

	switch (code->kind)
	{
		case IL_LABEL:
		case IL_JUMP:
		case IL_JUMP_TRUE:
		case IL_JUMP_FALSE:
			debug("#    label : %s", code->label_str);
			break ;
		case IL_DEF_VAR_LOCAL:
		case IL_DEF_VAR_LOCAL_ARRAY:
		case IL_VAR_LOCAL:
		case IL_VAR_LOCAL_ADDR:
			debug("#    name : %s",
			 my_strndup(code->var_local->name, code->var_local->name_len));
			break ;
		case IL_VAR_GLOBAL:
		case IL_VAR_GLOBAL_ADDR:
			debug("#    name : %s",
			 my_strndup(code->var_global->name, code->var_global->name_len));
			break ;
		case IL_ADD:
		case IL_SUB:
		case IL_MUL:
		case IL_DIV:
		case IL_MOD:
		case IL_EQUAL:
		case IL_NEQUAL:
		case IL_LESS:
		case IL_LESSEQ:
		case IL_BITWISE_AND:
		case IL_BITWISE_OR:
		case IL_BITWISE_XOR:
		case IL_BITWISE_NOT:
		case IL_SHIFT_LEFT:
		case IL_SHIFT_RIGHT:
			debug("#    type : %s", get_type_name(code->type));
			break ;
		case IL_CAST:
			debug("#    op : %s -> %s", get_type_name(code->cast_from), get_type_name(code->cast_to));
			break ;
		case IL_PUSH_NUM:
			debug("#    num : %d", code->number_int);
			break ;
		case IL_FUNC_PROLOGUE:
		case IL_FUNC_EPILOGUE:
		case IL_DEF_VAR_END:
		case IL_PUSH_AGAIN:
		case IL_PUSH_FLOAT:
		case IL_POP:
		case IL_STR_LIT:
		case IL_CALL_START:
		case IL_CALL_ADD_ARG:
		case IL_CALL_EXEC:
		case IL_MACRO_VASTART:
		case IL_LOAD:
		case IL_MEMBER:
		case IL_MEMBER_ADDR:
		case IL_MEMBER_PTR:
		case IL_MEMBER_PTR_ADDR:
		case IL_ASSIGN:
			break ;
	}
}

static t_il	*append_il(t_ilkind kind)
{
	t_il	*tmp;

	tmp				= calloc(1, sizeof(t_il));
	tmp->kind		= kind;
	tmp->next		= NULL;
	tmp->ilid_unique= ILID_UNIQUE++;
	tmp->label_str	= NULL;

	if (g_il == NULL)
		g_il = tmp;
	else
	{
		tmp->before = g_il_last;
		g_il_last->next = tmp;
	}
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

/*
static t_il	*append_il_pushflonum(float i)
{
	t_il	*code;
	code				= append_il(IL_PUSH_FLOAT);
	code->number_float	= i;
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
	code			= append_il(IL_JUMP_FALSE);
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
	code			= append_il(IL_JUMP_TRUE);
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

	for (i = node->funccall_argcount - 1; i >= 0; i--)
		translate_node(node->funccall_args[i]);

	code					= append_il(IL_CALL_START);
	code->funccall_callee	= node->funcdef;

	for (i = 0;	i < node->funccall_argcount; i++)
	{
		code					= append_il(IL_CALL_ADD_ARG);
		code->funccall_callee	= node->funcdef;
		code->type				= type_array_to_ptr(node->funccall_args[i]->type);
	}

	code					= append_il(IL_CALL_EXEC);
	code->funccall_callee	= node->funcdef;
}

static void	translate_return(t_node *node)
{
	t_il	*code;

	if (node->lhs != NULL)
		translate_node(node->lhs);

	code			= append_il(IL_JUMP);
	code->label_str	= get_function_epi_label(node->funcdef->name, node->funcdef->name_len);

	// push数を合わせるためのpushnum
	if (node->lhs == NULL)
		append_il_pushnum(0);
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
	// 中身が空なら0をpush
	else
	{
		append_il_pushnum(100);
	}
}

// lhs ? rhs : els
// ifのpopしない版
static void	translate_conditional_op(t_node *node)
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

	// 0ならジャンプ
	code			= append_il(IL_JUMP_TRUE);
	code->label_str	= get_label_str(lelse);

	// trueなら実行してlendにジャンプ
	translate_node(node->rhs);

	code			= append_il(IL_JUMP);
	code->label_str	= get_label_str(lend);

	// スタックの調整用pop
	append_il_pop(node->rhs->type);

	// false
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lelse);

	translate_node(node->els);

	// end label
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);
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

	// 0ならelseにジャンプ
	code			= append_il(IL_JUMP_TRUE);
	code->label_str	= get_label_str(lelse);

	// trueなら実行してlendにジャンプ
	translate_node(node->rhs);
	append_il_pop(node->rhs->type);

	code			= append_il(IL_JUMP);
	code->label_str	= get_label_str(lend);

	// false
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lelse);

	// else, else ifを実行
	// analyzeでelse ifを消したい
	if (node->elsif != NULL)
	{
		translate_node(node->elsif);
		append_il_pop(node->elsif->type);
	}
	else if (node->els != NULL)
	{
		translate_node(node->els);
		append_il_pop(node->els->type);
	}

	// end label
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);

	append_il_pushnum(40);
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

	code			= append_il(IL_JUMP_TRUE);
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

	append_il_pushnum(10);
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

	// 0ではないならlbeginにジャンプ
	code			= append_il(IL_JUMP_TRUE);
	code->label_str	= get_label_str(lbegin);

	// end
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);

	append_il_pushnum(20);
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

		// 0ならlendにジャンプ
		code			= append_il(IL_JUMP_TRUE);
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

	append_il_pushnum(30);
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

	// 保存先のアドレスをpush
	code			= append_il(IL_VAR_LOCAL_ADDR);
	code->var_local	= node->switch_save;

	// switch (lhs)
	translate_node(node->lhs);

	// 結果を保存してpop
	code			= append_il(IL_ASSIGN);
	code->type		= node->lhs->type; // TODO これint?
	append_il_pop(node->lhs->type);

	// if
	for (cases = node->block_sbdata->cases; cases != NULL; cases = cases->next)
	{
		code			= append_il(IL_VAR_LOCAL);
		code->var_local	= node->switch_save;

		append_il_pushnum(cases->value);

		code		= append_il(IL_EQUAL);
		code->type	= node->lhs->type;

		code			= append_il(IL_JUMP_TRUE);
		code->label_str	= get_label_str(cases->label);
	}

	// defaultかendに飛ばす
	code			= append_il(IL_JUMP);
	if (node->switch_has_default)
		code->label_str	= get_label_str(lbegin);
	else
		code->label_str	= get_label_str(lend);

	translate_node(node->rhs);
	append_il_pop(node->rhs->type);

	//end
	code			= append_il(IL_LABEL);
	code->label_str	= get_label_str(lend);

	append_il_pushnum(50);
}
*/

static void translate_node(t_ir_stmt_base* node)
{
	switch(node->kind)
	{
		case IR_NUMBER:
		{
			append_il_pushnum(node->val);
			return ;
		}
		case IR_RETURN:
		{
			translate_return(node);
			return ;
		}
	}
}

static void	translate_func(t_ir_func *func)
{
	t_il	*code;
	t_lvar	*lvar;
	char	*label_name;

	label_name = my_strndup(func->def->name, func->def->name_len);

	code						= append_il(IL_LABEL);
	code->label_str				= label_name;
	code->label_is_deffunc		= true;
	code->label_is_static_func	= func->def->is_static;

	code				= append_il(IL_FUNC_PROLOGUE);
	code->deffunc_def	= func->def;

	for (lvar = func->def->locals; lvar != NULL; lvar = lvar->next)
	{
		code = append_il(IL_DEF_VAR_LOCAL);
		code->var_local = lvar;
	}
	append_il(IL_DEF_VAR_END);

	translate_node(func->stmt);

	code			= append_il(IL_LABEL);
	code->label_str	= get_function_epi_label(func->def->name, func->def->name_len);

	code				= append_il(IL_FUNC_EPILOGUE);
	code->type			= func->type_return;
	code->deffunc_def	= func;
}

void	translate_il(void)
{
	int				i;
	t_funcil_pair	*pair;

	for (i = 0; g_ir_funcs[i] != NULL; i++)
	{
		g_il = NULL;
		translate_func(g_ir_funcs[i]);

		pair = calloc(1, sizeof(t_funcil_pair));
		pair->func = g_ir_funcs[i];
		pair->code = g_il;
		pair->next = g_func_ils;
		g_func_ils = pair;
	}
}
