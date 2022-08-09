#include "9cc.h"
#include "stack.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

t_lvar		*append_lvar(t_deffunc *func, char *name, int name_len, t_type *type, bool is_arg);
t_lvar		*append_dummy_lvar(t_deffunc *func, t_type *type);

static t_node	*cast(t_node *node, t_type *to);

static t_node	*analyze_var_def(t_node *node);
static t_node	*analyze_var(t_node *node);
static t_node	*analyze_deref(t_node *node);
static t_node	*analyze_addr(t_node *node);
static t_node	*analyze_member_value(t_node *node);
static t_node	*analyze_member_value_ptr(t_node *node);
static t_node	*analyze_call(t_node *node);
static t_node	*analyze_bitwise_not(t_node *node);
static void		analyze_replace_lval(t_node *node);
static t_node	*analyze_mul(t_node *node);
static t_node	*analyze_add(t_node *node);
static t_node	*analyze_shift(t_node *node);
static t_node	*analyze_relational(t_node *node);
static t_node	*analyze_equality(t_node *node);
static t_node	*analyze_bitwise_and(t_node *node);
static t_node	*analyze_bitwise_xor(t_node *node);
static t_node	*analyze_bitwise_or(t_node *node);
static t_node	*analyze_conditional_op(t_node *node);
static t_node	*analyze_assign(t_node *node);
static t_node	*analyze_if(t_node *node);
static t_node	*analyze_dowhile(t_node *node);
static t_node	*analyze_while(t_node *node);
static t_node	*analyze_for(t_node *node);
static t_node	*analyze_switch(t_node *node);
static t_node	*analyze_case(t_node *node);
static t_node	*analyze_break(t_node *node);
static t_node	*analyze_continue(t_node *node);
static t_node	*analyze_default(t_node *node);
static t_node	*analyze_node(t_node *node);
static void		analyze_func(t_deffunc *func);
static void		analyze_global_var(t_defvar *def);


// main
extern t_deffunc		*g_func_defs[1000];
extern t_deffunc		*g_func_protos[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals[1000];
extern t_defstruct		*g_struct_defs[1000];
extern t_defenum		*g_enum_defs[1000];
extern t_defunion		*g_union_defs[1000];
extern t_deffunc		*g_func_now;
extern t_linked_list	*g_type_alias;


static t_stack		*sbstack;
static t_labelstack	*sbdata_new(bool isswitch, int start, int end);
static t_labelstack	*sb_forwhile_start(int startlabel, int endlabel);
static t_labelstack	*sb_switch_start(t_type *type, int endlabel, int defaultLabel);
static t_labelstack	*sb_end(void);
static t_labelstack	*sb_peek(void);
static t_labelstack	*sb_search(bool	isswitch);
static t_switchcase	*add_switchcase(t_labelstack *sbdata, int number);

static t_labelstack	*sbdata_new(bool isswitch, int start, int end)
{
	t_labelstack	*tmp;

	tmp = (t_labelstack *)calloc(1, sizeof(t_labelstack));
	tmp->isswitch = isswitch;
	tmp->startLabel = start;
	tmp->endLabel = end;

	tmp->type = NULL;
	tmp->cases = NULL;
	tmp->defaultLabel = -1;
	return (tmp);
}

static t_labelstack	*sb_forwhile_start(int startlabel, int endlabel)
{
	t_labelstack	*tmp;

	tmp = sbdata_new(false, startlabel, endlabel);
	stack_push(&sbstack, tmp);
	return (tmp);
}

static t_labelstack	*sb_switch_start(t_type *type, int endlabel, int defaultLabel)
{
	t_labelstack	*tmp;

	tmp = sbdata_new(true, -1, endlabel);
	tmp->type = type;
	tmp->defaultLabel = defaultLabel;
	stack_push(&sbstack, tmp);
	return (tmp);
}


static t_labelstack	*sb_end(void)
{
	t_labelstack	*result;

	result = stack_pop(&sbstack);
	return (result);
}

static t_labelstack	*sb_peek(void)
{
	return (t_labelstack *)stack_peek(sbstack);
}

static t_labelstack	*sb_search(bool	isswitch)
{
	t_stack			*tmp;
	t_labelstack	*data;

	for (tmp = sbstack; tmp != NULL; tmp = tmp->prev)
	{
		data = (t_labelstack *)tmp->data;
		if (data->isswitch == isswitch)
			return (data);
	}
	return (NULL);
}

static t_switchcase	*add_switchcase(t_labelstack *sbdata, int number)
{
	t_switchcase	*tmp;

	tmp = (t_switchcase *)malloc(sizeof(t_switchcase));
	tmp->value = number;
	tmp->label = 0;
	tmp->next = sbdata->cases;
	sbdata->cases = tmp;

	//TODO 被りチェック
	return (tmp);
}


// parse.cからのコピー
static t_node	*cast(t_node *node, t_type *to)
{
	node = new_node(ND_CAST, node, NULL, node->analyze_source);
	node->type = to;
	return (node);
}





static t_node *analyze_var_def(t_node *node)
{
	t_lvar	*lvar;

	if (!is_declarable_type(node->type))
		error_at(node->analyze_source, "%s型の変数は宣言できません", get_type_name(node->type));

	lvar = append_lvar(g_func_now, node->analyze_var_name, node->analyze_var_name_len, node->type, false);
	node->lvar = lvar;

	if (node->lvar_assign != NULL)
	{
		node->kind = ND_VAR_LOCAL;
		node = new_node(ND_ASSIGN, node, node->lvar_assign, node->analyze_source);
		node = analyze_node(node);
	}
	else
		node->kind = ND_NONE;
	return (node);
}

static t_node *analyze_var(t_node *node)
{
	t_lvar		*lvar;
	t_defvar	*defvar;
	t_node		*assign;

	lvar = find_lvar(g_func_now, node->analyze_var_name, node->analyze_var_name_len);
	if (lvar != NULL)
	{

		node->kind			= ND_VAR_LOCAL;
		node->lvar			= lvar;
		node->type			= lvar->type;

		if (node->lvar_assign == NULL)
			return (node);

		assign				= analyze_node(node->lvar_assign);
		node->lvar_assign	= NULL;
		node				= new_node(ND_ASSIGN, node, assign, node->analyze_source);
		return (analyze_node(node));
	}

	// グローバル変数
	defvar = find_global(node->analyze_var_name, node->analyze_var_name_len);
	if (defvar != NULL)
	{
		node->kind			= ND_VAR_GLOBAL;
		node->var_global	= defvar;
		node->type			= defvar->type;
		return (node);
	}

	error_at(node->analyze_var_name, "%sが定義されていません", my_strndup(node->analyze_var_name, node->analyze_var_name_len));
	return (NULL);
}

// *lhs
static t_node	*analyze_deref(t_node *node)
{
	node->lhs = analyze_node(node->lhs);

	if (!is_pointer_type(node->lhs->type))
		error_at(node->analyze_source,"%sに演算子を適用できません", get_type_name(node->lhs->type));

	node->type = node->lhs->type->ptr_to;
	return (node);
}

// &lhs
static t_node	*analyze_addr(t_node *node)
{
	node->lhs = analyze_node(node->lhs);
	analyze_replace_lval(node->lhs);
	node->type = new_type_ptr_to(node->lhs->type);
	return (node->lhs);
}

static t_node	*analyze_member_value_ptr(t_node *node)
{
	t_member	*elem;

	node->lhs = analyze_node(node->lhs);
	if (!can_use_arrow(node->lhs->type))
		error_at(node->analyze_source, "%sに->を適用できません", get_type_name(node->lhs->type));

	elem = get_member_by_name(node->lhs->type->ptr_to,
							node->analyze_member_name,
							node->analyze_member_name_len);
	if (elem == NULL)
		error_at(node->analyze_source, "識別子が存在しません",
				my_strndup(node->analyze_member_name, node->analyze_member_name_len));

	node->elem = elem;
	node->type = elem->type;
	return (node);
}

static t_node	*analyze_member_value(t_node *node)
{
	t_member	*elem;

	node->lhs = analyze_node(node->lhs);
	if (!can_use_dot(node->lhs->type))
		error_at(node->analyze_source, "%sに.を適用できません", get_type_name(node->lhs->type));

	elem = get_member_by_name(node->lhs->type,
							node->analyze_member_name,
							node->analyze_member_name_len);
	if (elem == NULL)
		error_at(node->analyze_source, "識別子が存在しません",
				my_strndup(node->analyze_member_name, node->analyze_member_name_len));

	node->elem = elem;
	node->type = elem->type;
	return (node);
}

static t_node	*analyze_call(t_node *node)
{
	int		i;
	t_node	*argnode;

	// set caller
	node->funccall_caller = g_func_now;

	// is va_start macro?
	if (node->analyze_funccall_name_len == 8
	&& strncmp(node->analyze_funccall_name, "va_start", 8) == 0)
	{
		node->kind = ND_CALL_MACRO_VA_START;
		
		// analyze arguments
		for (i = 0; i < node->funccall_argcount; i++)
			node->funccall_args[i] = analyze_node(node->funccall_args[i]);

		if (node->funccall_argcount == 0)
			error_at(node->analyze_source, "va_startに引数がありません");

		analyze_replace_lval(node->funccall_args[0]);
		return (node);
	}

	// function exist?
	if (node->funcdef == NULL)
		error_at(node->analyze_source, "関数%sがみつかりません", my_strndup(node->analyze_funccall_name, node->analyze_funccall_name_len));

	// set type
	node->type = node->funcdef->type_return;

	// analyze arguments
	for (i = 0; i < node->funccall_argcount; i++)
	{
		node->funccall_args[i] = analyze_node(node->funccall_args[i]);
	}

	// 引数の数を比較する
	if (node->funcdef->is_zero_argument && node->funccall_argcount != 0)
		error_at(node->analyze_source, "関数%sの引数の数が一致しません\n expected : %d\n actual : %d",
				my_strndup(node->funcdef->name, node->funcdef->name_len),
				node->funcdef->argcount, node->funccall_argcount);
	else
	{
		if (!node->funcdef->is_variable_argument && node->funccall_argcount != node->funcdef->argcount)
			error_at(node->analyze_source, "関数%sの引数の数が一致しません\n expected : %d\n actual : %d",
						my_strndup(node->funcdef->name, node->funcdef->name_len),
						node->funcdef->argcount, node->funccall_argcount);
		if (node->funcdef->is_variable_argument && node->funccall_argcount < node->funcdef->argcount)
			error_at(node->analyze_source, "関数%sの引数の数が一致しません\n expected : %d\n actual : %d",
						my_strndup(node->funcdef->name, node->funcdef->name_len),
						node->funcdef->argcount, node->funccall_argcount);
	}

	// 型の確認
	for (i = 0; i < node->funccall_argcount; i++)
	{
		argnode			= node->funccall_args[i];
		argnode->type	= type_array_to_ptr(argnode->type);

		if (i < node->funcdef->argcount)
		{
			if (!type_equal(node->funcdef->argument_types[i], argnode->type))
			{
				// 暗黙的なキャストの確認
				if (!type_can_cast(argnode->type, node->funcdef->argument_types[i], false))
					error_at(argnode->analyze_source, "関数%sの引数(%s)の型が一致しません\n %s と %s",
							my_strndup(node->funcdef->name, node->funcdef->name_len),
							my_strndup(node->funcdef->argument_names[i], node->funcdef->argument_name_lens[i]),
							get_type_name(node->funcdef->argument_types[i]),
							get_type_name(argnode->type));
				node->funccall_args[i] = analyze_node(cast(argnode, node->funcdef->argument_types[i]));
			}
		}
	}

	return (node);
}

static t_node *analyze_bitwise_not(t_node *node)
{
	node->lhs = analyze_node(node->lhs);

	if (!is_integer_type(node->lhs->type))
		error_at(node->analyze_source, "整数ではない型に~を適用できません");

	node->type = node->lhs->type;
	return (node);
}

// kindとtypeを変える副作用があるので、関数の最後で実行する
static void	analyze_replace_lval(t_node *node)
{
	// TODO 文字列も?
	if (node->kind == ND_VAR_LOCAL)
		node->kind = ND_VAR_LOCAL_ADDR;
	else if (node->kind == ND_VAR_GLOBAL)
		node->kind = ND_VAR_GLOBAL_ADDR;
	else if (node->kind == ND_DEREF)
		node->kind = ND_DEREF_ADDR;
	else if (node->kind == ND_MEMBER_VALUE)
		node->kind = ND_MEMBER_VALUE_ADDR;
	else if (node->kind == ND_MEMBER_PTR_VALUE)
		node->kind = ND_MEMBER_PTR_VALUE_ADDR;
	else
		error_at(node->analyze_source, "左辺値ではありません");

	node->type = new_type_ptr_to(node->type);
}

static t_node	*analyze_mul(t_node *node)
{
	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);

	if (!is_integer_type(node->lhs->type))
		error_at(node->analyze_source, "* か / か %% の左辺が整数ではありません", get_type_name(node->lhs->type));

	if (!is_integer_type(node->rhs->type))
		error_at(node->analyze_source, "* か / か %% の右辺が整数ではありません", get_type_name(node->rhs->type));

	node->type = new_primitive_type(TY_INT);

	// assignは置き換える
	if (node->kind == ND_COMP_MUL || node->kind == ND_COMP_DIV || node->kind == ND_COMP_MOD)
		analyze_replace_lval(node->lhs);	
	return (node);
}

static t_node	*analyze_add(t_node *node)
{
	t_type	*l;
	t_type	*r;
	int		size;

	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);

	l = node->lhs->type;
	r = node->rhs->type;

	// 両方ともポインタ
	if (is_pointer_type(l) && is_pointer_type(r))
	{
		if (!type_equal(l, r))
			error_at(node->analyze_source, "型が一致しないポインタ型どうしの加減算はできません");
		node->type = new_primitive_type(TY_INT); //size_t

		size = get_type_size(l->ptr_to);
		if (size == 0 || size > 1)
		{
			if (size == 0)
				fprintf(stderr, "WARNING : サイズ0の型のポインタ型どうしの加減算は未定義動作です\n");

			node		= new_node(ND_DIV, node, new_node_num(size, node->analyze_source), node->analyze_source);
			node->type	= new_primitive_type(TY_INT);
			node		= analyze_node(node);
		}
	}
	else
	{
		// TODO int += ptr;
		// ポインタと整数の演算
		if (is_pointer_type(l) && is_integer_type(r))
		{
			node->type = l;

			// 右辺を掛け算に置き換える
			node->rhs = new_node(ND_MUL, node->rhs, new_node_num(get_array_align_size(l->ptr_to), node->analyze_source), node->analyze_source);
			node->rhs->type = new_primitive_type(TY_INT);
			node->rhs->analyze_source = node->rhs->lhs->analyze_source;
			node->rhs = analyze_node(node->rhs);
		}
		// ポインタと整数の演算
		else if (is_pointer_type(r) && is_integer_type(l))
		{
			node->type = r;

			// 左辺を掛け算に置き換える
			node->lhs = new_node(ND_MUL, node->lhs, new_node_num(get_array_align_size(r->ptr_to), node->analyze_source), node->analyze_source);
			node->lhs->type = new_primitive_type(TY_INT);
			node->lhs->analyze_source = node->lhs->lhs->analyze_source;
			node->lhs = analyze_node(node->lhs);
		}
		// 両方整数なら型の優先順位を考慮
		else if (is_integer_type(l) || is_integer_type(r))
		{
			// TODO とりあえずサイズの大きいほうの型にしている
			if (get_type_size(l) > get_type_size(r))
				node->type = l;
			else
				node->type = r;
		}
		else
			error_at(node->analyze_source, "演算子が%sと%sの間に定義されていません",
				get_type_name(node->lhs->type), get_type_name(node->rhs->type));
	}

	// assignは置き換える
	if (node->kind == ND_COMP_ADD || node->kind == ND_COMP_SUB)
		analyze_replace_lval(node->lhs);

	return (node);
}

static t_node *analyze_shift(t_node *node)
{
	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);

	// TODO 型チェック
	if (!is_integer_type(node->lhs->type)
		|| !is_integer_type(node->rhs->type))
		error_at(node->analyze_source, "整数型ではない型にシフト演算子を適用できません");

	// 左辺の型にする
	node->type = node->lhs->type;
	return (node);
}

static t_node	*analyze_relational(t_node *node)
{
	t_type	*l_to;
	t_type	*r_to;

	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);
	
	// 比較できるか確認
	if (!can_compared(node->lhs->type, node->rhs->type, &l_to, &r_to))
		error_at(node->analyze_source, "%sと%sを比較することはできません",
				get_type_name(node->lhs->type), get_type_name(node->rhs->type));

	// キャストする必要があるならキャスト
	if (!type_equal(node->lhs->type, l_to))
		node->lhs = analyze_node(cast(node->lhs, l_to));
	if (!type_equal(node->rhs->type, r_to))
		node->rhs = analyze_node(cast(node->rhs, r_to));

	node->type = new_primitive_type(TY_INT);
	return (node);
}

static t_node	*analyze_equality(t_node *node)
{
	t_type	*l_to;
	t_type	*r_to;

	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);
	
	// 比較できるか確認
	if (!can_compared(node->lhs->type, node->rhs->type, &l_to, &r_to))
		error_at(node->analyze_source, "%sと%sを比較することはできません",
				get_type_name(node->lhs->type), get_type_name(node->rhs->type));

	// キャストする必要があるならキャスト
	if (!type_equal(node->lhs->type, l_to))
		node->lhs = analyze_node(cast(node->lhs, l_to));
	if (!type_equal(node->rhs->type, r_to))
		node->rhs = analyze_node(cast(node->rhs, r_to));

	node->type = new_primitive_type(TY_INT);
	return (node);
}

static t_node	*analyze_bitwise_and(t_node *node)
{
	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);

	if (!is_integer_type(node->lhs->type)
		|| !is_integer_type(node->rhs->type))
		error_at(node->analyze_source, "整数型ではない型に&を適用できません");

	// TODO キャストしてから可能か考える
	if (!type_equal(node->rhs->type, node->lhs->type))
		error_at(node->analyze_source, "&の両辺の型が一致しません");

	node->type = node->rhs->type;
	return (node);
}

static t_node	*analyze_bitwise_xor(t_node *node)
{
	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);

	if (!is_integer_type(node->lhs->type)
		|| !is_integer_type(node->rhs->type))
		error_at(node->analyze_source, "整数型ではない型に^を適用できません");

	// TODO キャストしてから可能か考える
	if (!type_equal(node->rhs->type, node->lhs->type))
		error_at(node->analyze_source, "^の両辺の型が一致しません");

	node->type = node->rhs->type;
	return (node);
}

static t_node	*analyze_bitwise_or(t_node *node)
{
	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);

	if (!is_integer_type(node->lhs->type)
		|| !is_integer_type(node->rhs->type))
		error_at(node->analyze_source, "整数型ではない型に|を適用できません");

	// TODO キャストしてから可能か考える
	if (!type_equal(node->rhs->type, node->lhs->type))
		error_at(node->analyze_source, "|の両辺の型が一致しません");

	node->type = node->rhs->type;
	return (node);
}

// lhs && rhs
// lhs || rhs
static t_node	*analyze_conditional(t_node *node)
{
	// TODO 比較可能な型かチェックする
	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);
	node->type = new_primitive_type(TY_INT);
	return (node);
}

// lhs ? rhs : els
static t_node	*analyze_conditional_op(t_node *node)
{
	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);
	node->els = analyze_node(node->els);

	// TODO lhsがboolになれるかチェックする

	// TODO キャストできるかチェックする -> そもそもキャストするの？
	if (!type_equal(node->rhs->type, node->els->type))
		error_at(node->analyze_source, "条件演算子の型が一致しません");

	node->type = node->rhs->type;

	return (node);
}

static t_node	*analyze_assign(t_node *node)
{
	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);

	// 代入可能な型かどうか確かめる。
	if (node->lhs->type->ty == TY_VOID
	|| node->rhs->type->ty == TY_VOID)
		error_at(node->analyze_source, "voidを宣言、代入できません");

	if (!type_equal(node->rhs->type, node->lhs->type))
	{
		if (type_can_cast(node->rhs->type, node->lhs->type, false))
		{
			debug("assign (%s) <- (%s)",
					get_type_name(node->lhs->type),
					get_type_name(node->rhs->type));
			node->rhs = analyze_node(cast(node->rhs, node->lhs->type));
		}
		else
		{
			error_at(node->analyze_source, "左辺(%s)に右辺(%s)を代入できません",
					get_type_name(node->lhs->type),
					get_type_name(node->rhs->type));
		}
	}

	node->type = node->lhs->type;

	analyze_replace_lval(node->lhs);
	return (node);
}

// sizeof lhs
static t_node	*analyze_sizeof(t_node *node)
{
	t_node	*result;

	node->lhs = analyze_node(node->lhs);
	result = new_node_num(get_type_size(node->lhs->type), node->lhs->analyze_source);
	return (analyze_node(result));
}

// if (lhs)
//   rhs
// els
// elsif
static t_node	*analyze_if(t_node *node)
{
	node->lhs = analyze_node(node->lhs);
	node->rhs = analyze_node(node->rhs);

	if (node->els != NULL)
		node->els = analyze_node(node->els);
	if (node->elsif != NULL)
		node->elsif = analyze_node(node->elsif);

	return (node);
}

// while (lhs) rhs
static t_node	*analyze_while(t_node *node)
{
	t_labelstack	*data;

	node->lhs = analyze_node(node->lhs);

	data = sb_forwhile_start(-1, -1);
	if (node->rhs != NULL)
		node->rhs = analyze_node(node->rhs);
	sb_end();

	node->block_sbdata = data;
	return (node);
}

// do lhs while (rhs)
static t_node	*analyze_dowhile(t_node *node)
{
	t_labelstack	*data;

	data = sb_forwhile_start(-1, -1);
	node->lhs = analyze_node(node->lhs);
	sb_end();

	node->block_sbdata = data;
	node->rhs = analyze_node(node->rhs);
	return (node);
}

// for (0; 1; 2) lhs
static t_node	*analyze_for(t_node *node)
{
	t_labelstack	*data;
	int		i;

	for (i = 0; i < 3; i++)
		if (node->for_expr[i] != NULL)
			node->for_expr[i] = analyze_node(node->for_expr[i]);
	
	data = sb_forwhile_start(-1, -1);
	if (node->lhs != NULL)
		node->lhs = analyze_node(node->lhs);
	sb_end();

	node->block_sbdata = data;
	return (node);
}

// switch (lhs) rhs
static t_node	*analyze_switch(t_node *node)
{
	t_labelstack	*data;

	node->lhs = analyze_node(node->lhs);

	// TODO キャストも
	if (!is_integer_type(node->lhs->type))
		error_at(node->lhs->analyze_source, "整数型以外の型で分岐することはできません");

	sb_switch_start(node->lhs->type, -1, -1);
	node->rhs = analyze_node(node->rhs);
	data = sb_end();

	node->block_sbdata = data;
	node->switch_has_default = data->defaultLabel != -1;

	// lhsの保存用
	node->switch_save = append_dummy_lvar(g_func_now, node->lhs->type);
	return (node);
}

static t_node	*analyze_case(t_node *node)
{
	t_labelstack	*data;

	data = sb_search(true);
	if (data == NULL)
		error_at(node->analyze_source, "caseに対応するswitchがありません");

	// TODO 被りチェック
	node->case_label = add_switchcase(data, node->val);
	node->block_sbdata = data;
	return (node);
}

static t_node	*analyze_break(t_node *node)
{
	t_labelstack	*data;

	data = sb_peek();
	if (data == NULL)
		error_at(node->analyze_source, "breakに対応するstatementがありません");
	node->block_sbdata = data;
	return (node);
}

static t_node	*analyze_continue(t_node *node)
{
	t_labelstack	*data;

	data = sb_search(false);
	if (data == NULL)
		error_at(node->analyze_source, "continueに対応するstatementがありません");
	node->block_sbdata = data;
	return (node);
}

static t_node	*analyze_default(t_node *node)
{
	t_labelstack	*data;

	data = sb_search(true);
	if (data == NULL)
		error_at(node->analyze_source, "defaultに対応するstatementがありません");
	if (data->defaultLabel != -1)
		error_at(node->analyze_source, "defaultが2個以上あります");
	data->defaultLabel = 1; // TODO これはgenで決まる(後でここに持ってくる)
	node->block_sbdata = data;
	return (node);
}

static t_node	*analyze_node(t_node *node)
{
	if (node->is_analyzed)
		return (node);
	node->is_analyzed = true;

	switch (node->kind)
	{
		case ND_NONE:
		case ND_NUM:
			return (node);
		case ND_STR_LITERAL:
		{
			node->type = new_type_array(new_primitive_type(TY_CHAR));
			node->type->array_size = node->def_str->len;
			// TODO 実際のサイズではない
			//fprintf(stderr, "array : %d\n", node->def_str->len);
			return (node);
		}
		case ND_BLOCK:
		{
			if (node->lhs != NULL)
			{
				node->lhs = analyze_node(node->lhs);
				node->type = node->lhs->type;
			}
			else
				node->type = new_primitive_type(TY_INT); // lhsがNULLならとりあえず型をINTにする

			if (node->rhs != NULL)
				node->rhs = analyze_node(node->rhs);

			return (node);
		}
		case ND_BITWISE_NOT:
			return (analyze_bitwise_not(node));
		case ND_ADD_UNARY:
		{
			node = analyze_node(node->lhs);
			if (!is_integer_type(node->type))
				error_at(node->analyze_source, "%sに+を適用できません", get_type_name(node->type));
			return (node);
		}
		case ND_SUB_UNARY:
		{
			node = analyze_node(node->lhs);
			if (!is_integer_type(node->type))
				error_at(node->analyze_source, "%sに-を適用できません", get_type_name(node->type));
			node = new_node(ND_SUB, new_node_num(0, node->analyze_source), node, node->analyze_source);
			return (analyze_node(node));
		}
		case ND_ADD:
		case ND_SUB:
		case ND_COMP_ADD:
		case ND_COMP_SUB:
			return (analyze_add(node));
		case ND_MUL:
		case ND_DIV:
		case ND_MOD:
		case ND_COMP_MUL:
		case ND_COMP_DIV:
		case ND_COMP_MOD:
			return (analyze_mul(node));
		case ND_BITWISE_XOR:
			return (analyze_bitwise_xor(node));
		case ND_BITWISE_AND:
			return (analyze_bitwise_and(node));
		case ND_BITWISE_OR:
			return (analyze_bitwise_or(node));
		case ND_PARENTHESES:
		{
			node->lhs = analyze_node(node->lhs);
			return (node->lhs);
		}
		case ND_SHIFT_LEFT:
		case ND_SHIFT_RIGHT:
			return (analyze_shift(node));
		case ND_EQUAL:
		case ND_NEQUAL:
			return (analyze_equality(node));
		case ND_LESS:
		case ND_LESSEQ:
			return (analyze_relational(node));
		case ND_COND_AND:
		case ND_COND_OR:
			return (analyze_conditional(node));
		case ND_ASSIGN:
			return (analyze_assign(node));
		case ND_VAR_DEF:
			return (analyze_var_def(node));
		case ND_ANALYZE_VAR:
			return (analyze_var(node));
		case ND_DEREF:
			return (analyze_deref(node));
		case ND_ADDR:
			return (analyze_addr(node));
		case ND_MEMBER_PTR_VALUE:
			return (analyze_member_value_ptr(node));
		case ND_MEMBER_VALUE:
			return (analyze_member_value(node));
		case ND_CALL:
			return (analyze_call(node));
		case ND_CAST:
		{
			node->lhs = analyze_node(node->lhs);
			if (!type_can_cast(node->lhs->type, node->type, true))
				error_at(node->analyze_source, "%sを%sにキャストできません",
						get_type_name(node->lhs->type), get_type_name(node->type));
			return (node);
		}
		case ND_RETURN:
		{
			if (node->lhs != NULL)
				node->lhs = analyze_node(node->lhs);
			// TODO 型チェック, キャスト
			node->funcdef = g_func_now;
			return (node);
		}
		case ND_SIZEOF:
			return (analyze_sizeof(node));
		case ND_COND_OP:
			return (analyze_conditional_op(node));
		case ND_IF:
			return (analyze_if(node));
		case ND_WHILE:
			return (analyze_while(node));
		case ND_DOWHILE:
			return (analyze_dowhile(node));
		case ND_FOR:
			return (analyze_for(node));
		case ND_SWITCH:
			return (analyze_switch(node));
		case ND_CASE:
			return (analyze_case(node));
		case ND_BREAK:
			return (analyze_break(node));
		case ND_CONTINUE:
			return (analyze_continue(node));
		case ND_DEFAULT:
			return (analyze_default(node));
		default:
			fprintf(stderr, "# kind %d\n", node->kind);
			break;
	}
	return (node);
}

static void	analyze_func(t_deffunc *func)
{
	int		i;
	char	*name;
	int		name_len;
	t_type	*type;

	t_node	*tmp_loop;
	t_node	*tmp_ret;

	g_func_now = func;

	// TODO 引数の名前の被りチェック
	// check argument
	for (i = 0; i < func->argcount; i++)
	{
		name		= func->argument_names[i];
		name_len	= func->argument_name_lens[i];
		type		= func->argument_types[i];

		// if array type, change type to ptr
		func->argument_types[i] = type_array_to_ptr(type);

		if (!is_declarable_type(type))
			error_at(name, "宣言できない型の変数です");

		// 引数を格納する変数を作成
		append_lvar(func, name, name_len, type, true);
	}

	// TODO 関数名の被りチェック
	if (!func->is_prototype)
		func->stmt = analyze_node(func->stmt);

	// main関数
	// TODO 引数と返り値の型のチェック
	if (func->name_len == 4 && strncmp(func->name, "main", 4) == 0)
	{
		if (!func->is_prototype)
		{
			if (func->stmt->kind == ND_BLOCK)
			{
				tmp_ret = new_node(ND_BLOCK, new_node(ND_RETURN, new_node_num(0, func->name), NULL, func->name), NULL, func->name);
				tmp_ret = analyze_node(tmp_ret);

				for (tmp_loop = func->stmt; tmp_loop->rhs != NULL; tmp_loop = tmp_loop->rhs);

				if (tmp_loop->lhs == NULL)
					tmp_loop->lhs = tmp_ret;
				else
					tmp_loop->rhs = tmp_ret;
			}
			else
			{
				tmp_ret = new_node(ND_BLOCK, new_node(ND_RETURN, new_node_num(0, func->name), NULL, func->name), NULL, func->name);
				func->stmt = new_node(ND_BLOCK, func->stmt, tmp_ret, func->name);
				func->stmt = analyze_node(func->stmt);
			}
		}	
	}

	g_func_now = NULL;
}

static void analyze_global_var(t_defvar *def)
{
	if (def->is_static && def->is_extern)
		error_at(def->name, "staticとexternは併用できません");
	if (!is_declarable_type(def->type))
		error_at(def->name, "宣言できない型の変数です");
}

static void	analyze_struct(t_defstruct *def)
{
	t_member	*mem;

	for (mem = def->members; mem != NULL; mem = mem->next)
	{
		if (!is_declarable_type(mem->type))
			error_at(mem->name, "宣言できない型です");
	}
}

static void	analyze_union(t_defunion *def)
{
	t_member	*mem;

	for (mem = def->members; mem != NULL; mem = mem->next)
	{
		if (!is_declarable_type(mem->type))
			error_at(mem->name, "宣言できない型です");
	}
}

void	analyze(void)
{
	int	i;

	for (i = 0; g_func_protos[i] != NULL; i++)
		analyze_func(g_func_protos[i]);
	for (i = 0; g_func_defs[i] != NULL; i++)
		analyze_func(g_func_defs[i]);
	for (i = 0; g_global_vars[i] != NULL; i++)
		analyze_global_var(g_global_vars[i]);
	for (i = 0; g_struct_defs[i] != NULL; i++)
		analyze_struct(g_struct_defs[i]);
	for (i = 0; g_union_defs[i] != NULL; i++)
		analyze_union(g_union_defs[i]);
}
