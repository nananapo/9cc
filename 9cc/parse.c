#include "9cc.h"
#include "parse.h"
#include "stack.h"

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define Env ParseResult

// Node
static Node	*expr(Env *env);
static Node *unary(Env *env);
static Node	*stmt(Env *env);
Node	*read_struct_block(Env *env, Token *ident);
static Node	*create_add(bool isadd, Node *lhs, Node *rhs, Token *tok);

// LVar
LVar		*find_lvar(Env *env, char *str, int len);
LVar		*create_local_var(Env *env, char *name, int len, Type *type, bool is_arg);


static Node	*get_function_by_name(Env *env, char *name, int len)
{
	int i = 0;
	while (env->func_defs[i])
	{
		Node *tmp = env->func_defs[i];
		if (tmp->flen == len && strncmp(tmp->fname, name, len) == 0)
			return tmp;
		i++;
	}

	i = 0;
	while (env->func_protos[i])
	{
		Node *tmp = env->func_protos[i];
		if (tmp->flen == len && strncmp(tmp->fname, name, len) == 0)
			return tmp;
		i++;
	}
	return NULL;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;

	// for
	node->for_init = NULL;
	node->for_if = NULL;
	node->for_next = NULL;

	// else
	node->els = NULL;
	
	// call & deffunc
	node->fname = NULL;
	node->args = NULL;

	node->type = NULL;
	return node;
}

Node*new_node_num(int val)
{
	Node *node = new_node(ND_NUM, NULL, NULL);
	node->val = val;

	//if (val < 127 && val > -128)
	//	node->type = new_primitive_type(CHAR);
	//else
		node->type = new_primitive_type(INT);
	return node;
}
















// リテラルを探す
static int	get_str_literal_index(Env *env, char *str, int len)
{
	t_str_elem	*tmp;

	tmp = env->str_literals;
	while (tmp != NULL)
	{
		if (len == tmp->len && strncmp(tmp->str, str, len) == 0)
			return (tmp->index);
		tmp = tmp->next;
	}

	tmp = calloc(1, sizeof(t_str_elem));
	tmp->str = str;
	tmp->len = len;
	if (env->str_literals == NULL)
		tmp->index = 0;
	else
		tmp->index = env->str_literals->index + 1;

	tmp->next = env->str_literals;
	env->str_literals = tmp;

	return (tmp->index);
}

static Node *call(Env *env, Token *tok)
{
	Node	*node;
	Node	*args;

	node  = new_node(ND_CALL, NULL, NULL);
	node->fname = tok->str;
	node->flen = tok->len;
	node->args = NULL;
	node->argdef_count = 0;

	args = NULL;

	if (!consume(env, ")"))
	{
		Node *arg = expr(env);
		args = arg;
		node->argdef_count = 1;

		for (;;)
		{	
			if (consume(env, ")"))
				break;
			if (!consume(env, ","))
				error_at(env->token->str, "トークンが,ではありません");
			
			arg = expr(env);
			arg->next = NULL;
			if (args == NULL)
				args = arg;
			else
			{
				for (Node *tmp = args; tmp; tmp = tmp->next)
				{
					if (tmp->next == NULL)
					{
						tmp->next = arg;
						break ;
					}
				}
			}
			node->argdef_count += 1;
		}
	}

	Node *refunc = get_function_by_name(env, node->fname, node->flen);
	// 関数定義が見つからない場合エラー
	if (refunc == NULL)
		error_at(env->token->str, "warning : 関数%sがみつかりません\n", strndup(node->fname, node->flen));

	// 引数の数を確認
	if (refunc->argdef_count != -1
		&& node->argdef_count != refunc->argdef_count)
		error_at(env->token->str, "関数%sの引数の数が一致しません\n expected : %d\n actual : %d", strndup(node->fname, node->flen), refunc->argdef_count, node->argdef_count);

	// 引数の型を比べる
	LVar *def = refunc->locals;

	for (int i = 0; i < node->argdef_count; i++)
	{
		// 型の確認
		if (!type_equal(def->type, args->type))
		{
			// 暗黙的なキャストの確認
			if (!type_can_cast(args->type, def->type, false))
				error_at(env->token->str, "関数%sの引数(%s)の型が一致しません\n %s と %s",
						strndup(node->fname, node->flen),
						strndup(def->name, def->len),
						get_type_name(def->type),
						get_type_name(args->type));

			Node *cast = new_node(ND_CAST, args, NULL);
			cast->type = def->type;
			cast->next = args->next;
			args = cast;
		}

		// きっとuse->localsは使われないので使ってしまう
		args->locals = def;

		// 格納
		if (node->args == NULL)
			node->args = args;
		else
		{
			Node *tmp = node->args;
			for (int j = 0; j < i - 1; j++)
				tmp = tmp->next;
			tmp->next = args;
		}

		// 進める
		def = def->next;
		args = args->next;
	}

	// 型を返り値の型に設定
	node->type = refunc->ret_type;

	return node;
}

// 後置インクリメント, デクリメント
static Node	*read_suffix_increment(Env *env, Node *node)
{
	if (consume(env, "++"))
	{
		node = create_add(true, node, new_node_num(1), env->token);
		node->kind = ND_COMP_ADD;
		node = create_add(false, node, new_node_num(1), env->token); // 1を引く
	}
	else if (consume(env, "--"))
	{
		node = create_add(false, node, new_node_num(1), env->token);
		node->kind = ND_COMP_SUB;
		node = create_add(true, node, new_node_num(1), env->token); // 1を足す
	}
	return (node);
}

// 添字によるDEREF
static Node	*read_deref_index(Env *env, Node *node)
{
	while (consume(env, "["))
	{
		Node	*add = new_node(ND_ADD, node, expr(env));

		// 左にポインタを寄せる
		if (is_pointer_type(add->rhs->type))
		{
			Node	*tmp;
			tmp = add->lhs;
			add->lhs = add->rhs;
			add->rhs = tmp;
		}

		if (!is_pointer_type(add->lhs->type))
			error_at(env->token->str, "ポインタ型ではありません");
		if (!is_integer_type(add->rhs->type))
			error_at(env->token->str, "添字の型が整数ではありません");
		add->type = add->lhs->type;

		node = new_node(ND_DEREF, add, NULL);
		node->type = node->lhs->type->ptr_to;

		if (!consume(env, "]"))
			error_at(env->token->str, "%s");
	}
	return read_suffix_increment(env, node);
}

static Node *primary(Env *env)
{
	Token	*tok;
	Node	*node;
	Type	*type_cast;

	// 括弧
	if (consume(env, "("))
	{
		// 型を読む
		type_cast = consume_type_before(env, false);
		
		// 括弧の中身が型ではないなら優先順位を上げる括弧
		if (type_cast == NULL)
		{
			node = new_node(ND_PARENTHESES, expr(env), NULL);
			node->type = node->lhs->type;
			expect(env, ")");
			return read_deref_index(env, node);
		}

		// 明示的なキャスト
		// TODO キャストの優先順位が違う
		expect(env, ")");
		node = new_node(ND_CAST, unary(env), NULL);
		if (!type_can_cast(node->lhs->type, type_cast, true))
			error_at(env->token->str, "%sを%sにキャストできません", get_type_name(node->lhs->type), get_type_name(type_cast));
		node->type = type_cast;
		return read_deref_index(env, node);
	}
	
	// identかcall
	tok = consume_ident(env);
	if (tok)
	{
		// call func
		if (consume(env, "("))
			node = call(env, tok);
		// use ident
		else
		{
			LVar	*lvar = find_lvar(env, tok->str, tok->len);
			if (lvar != NULL)
			{
				node = new_node(ND_LVAR, NULL, NULL);
				node->offset = lvar->offset;
				node->type = lvar->type;
			}
			else
			{
				Node *glovar = find_global(env, tok->str, tok->len);
				if (glovar != NULL)
				{
					node = new_node(ND_LVAR_GLOBAL, NULL, NULL);
					node->var_name = glovar->var_name;
					node->var_name_len = glovar->var_name_len;
					node->type = glovar->type;
				}
				else
					error_at(tok->str,
						"%sが定義されていません",
						strndup(tok->str, tok->len));
			}
		}
		return read_deref_index(env, node);
	}

	// string
	tok = consume_str_literal(env);
	if (tok)
	{
		node = new_node(ND_STR_LITERAL, NULL, NULL);
		node->str_index = get_str_literal_index(env, tok->str, tok->len);
		node->type = new_type_ptr_to(new_primitive_type(CHAR));
		return read_deref_index(env, node);
	}

	// char
	tok = consume_char_literal(env);
	if (tok)
	{
		node = new_node_num(get_char_to_int(tok->str, tok->strlen_actual));
		node->type = new_primitive_type(CHAR);
		return read_deref_index(env, node); // charの後ろに[]はおかしいけれど、とりあえず許容
	}

	// 数
	int number;
	if (!consume_number(env, &number))
		error_at(env->token->str, "数字が必要です");
	node = new_node_num(number);

	return read_deref_index(env, node);
}

static Node	*arrow_loop(Env *env, Node *node)
{
	Token				*ident;
	StructMemberElem	*elem;

	if (consume(env, "->"))
	{
		if (!is_pointer_type(node->type) || node->type->ptr_to->ty != STRUCT)
			error_at(env->token->str, "struct*ではありません");
		
		ident = consume_ident(env);

		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です");
		elem = struct_get_member(node->type->ptr_to->strct, ident->str, ident->len);
		if (elem == NULL)
			error_at(env->token->str, "識別子が存在しません", strndup(ident->str, ident->len));
		
		node = new_node(ND_STRUCT_PTR_VALUE, node, NULL);
		node->struct_elem = elem;
		node->type = elem->type;

		node = read_deref_index(env, node);

		return arrow_loop(env, node);
	}
	else if (consume(env, "."))
	{
		if (node->type->ty != STRUCT)
			error_at(env->token->str, "structではありません");

		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です");
		elem = struct_get_member(node->type->strct, ident->str, ident->len);
		if (elem == NULL)
			error_at(env->token->str, "識別子が存在しません", strndup(ident->str, ident->len));

		node = new_node(ND_STRUCT_VALUE, node, NULL);
		node->struct_elem = elem;
		node->type = elem->type;

		node = read_deref_index(env, node);

		return arrow_loop(env, node);
	}
	else
		return (node);
}

static Node	*arrow(Env *env)
{
	Node	*node;

	node = primary(env);
	return (arrow_loop(env, node));
}

static Node *unary(Env *env)
{
	Node	*node;

	if (consume(env, "+"))
	{
		node = unary(env);
		if (is_pointer_type(node->type))
			error_at(env->token->str, "ポインタ型に対してunary -を適用できません");
		return node;
	}
	else if (consume(env, "-"))
	{
		node = new_node(ND_SUB, new_node_num(0), unary(env));
		if (is_pointer_type(node->rhs->type))
			error_at(env->token->str, "ポインタ型に対してunary -を適用できません");
		node->type = node->rhs->type;
		return node;
	}
	else if (consume(env, "*"))
	{
		node = new_node(ND_DEREF, unary(env), NULL);
		if (!is_pointer_type(node->lhs->type))
			error_at(env->token->str,
					"ポインタではない型(%d)に対してunary *を適用できません",
					node->lhs->type->ty);
		node->type = node->lhs->type->ptr_to;	
		return node;
	}
	else if (consume(env, "&"))
	{
		node = new_node(ND_ADDR, unary(env), NULL);

		// 変数と構造体、ND_DEREFに対する&
		if (node->lhs->kind != ND_LVAR
		&& node->lhs->kind != ND_LVAR_GLOBAL
		&& node->lhs->kind != ND_STRUCT_VALUE
		&& node->lhs->kind != ND_STRUCT_PTR_VALUE
		&& node->lhs->kind != ND_DEREF) // TODO 文字列リテラルは？
			error_at(env->token->str, "変数以外に&演算子を適用できません Kind: %d", node->lhs->kind);

		node->type = new_type_ptr_to(node->lhs->type);
		return node;
	}
	else if (consume(env, "++"))
	{
		// TODO 左辺値かの検証はしていない
		node = create_add(true, unary(env), new_node_num(1), env->token);
		node->kind = ND_COMP_ADD;
		return (node);
	}
	else if (consume(env, "--"))
	{
		node = create_add(false, unary(env), new_node_num(1), env->token);
		node->kind = ND_COMP_SUB;
		return (node);
	}
	else if (consume(env, "!"))
	{
		// TODO 生成する関数を作る
		node = unary(env);
		node = new_node(ND_EQUAL, node, new_node_num(0));
		node->type = new_primitive_type(INT);
		return (node);
	}
	else if (consume_with_type(env, TK_SIZEOF))
	{
		node = unary(env);
		node = new_node_num(type_size(node->type));
		return node;
	}

	return arrow(env);
}

/*
 * type:
 * 0 : *
 * 1 : /
 * 2 : %
 */
static Node	*create_mul(int type, Node *lhs, Node *rhs, Token *tok)
{
	Node	*node;

	if (type == 0)
		node = new_node(ND_MUL, lhs, rhs);
	else if (type == 1)
		node = new_node(ND_DIV, lhs, rhs);
	else
		node = new_node(ND_MOD, lhs, rhs);

	if (!is_integer_type(node->lhs->type)
	|| !is_integer_type(node->rhs->type))
		error_at(tok->str, "ポインタ型に* か / を適用できません");

	node->type = new_primitive_type(INT);
	return (node);
}

static Node *mul(Env *env)
{
	Node *node = unary(env);
	for (;;)
	{
		if (consume(env, "*"))
			node = create_mul(0, node, unary(env), env->token);
		else if (consume(env, "/"))
			node = create_mul(1, node, unary(env), env->token);
		else if (consume(env, "%"))
			node = create_mul(2, node, unary(env), env->token);
		else
			return node;
	}
}

static Node	*create_add(bool isadd, Node *lhs, Node *rhs, Token *tok)
{
	Node	*node;
	Type	*l;
	Type	*r;

	if (isadd)
		node = new_node(ND_ADD, lhs, rhs);
	else
		node = new_node(ND_SUB, lhs, rhs);

	l = node->lhs->type;
	r = node->rhs->type;

	// 左辺をポインタにする
	if (is_pointer_type(r))
	{
		Type	*tmp;
		tmp = l;
		l = r;
		r = tmp;
	}

	// 両方ともポインタ
	if (is_pointer_type(l)
	&& is_pointer_type(r))
	{
		if (!type_equal(l, r))
			error_at(tok->str, "型が一致しないポインタ型どうしの加減算はできません");
		node->type = new_primitive_type(INT);// TODO size_tにする

		int size = type_size(l->ptr_to);
		if (size == 0 || size > 1)
		{
			if (size == 0)
				fprintf(stderr, "WARNING : サイズ0の型のポインタ型どうしの加減算は未定義動作です");
			node = new_node(ND_DIV, node, new_node_num(size));
			node->type = new_primitive_type(INT);
		}
		return (node);
	}

	// ポインタと整数の演算
	if (is_pointer_type(l)
	&& is_integer_type(r))
	{
		node->type = l;
		return (node);
	}

	// 両方整数なら型の優先順位を考慮
	if (is_integer_type(l)
	|| is_integer_type(r))
	{
		// Intを左に寄せる
		if (r->ty == INT)
		{
			Type	*tmp;
			tmp = l;
			l = r;
			r = tmp;
		}
		// 左辺の型にする
		node->type = l;
		return (node);
	}

	error_at(tok->str, "演算子 +, - が%dと%dの間に定義されていません",
			node->lhs->type->ty, node->rhs->type->ty);
	return (NULL);
}

static Node *add(Env *env)
{
	Node *node = mul(env);
	for (;;)
	{
		if (consume(env, "+"))
			node = create_add(true, node, mul(env), env->token);
		else if (consume(env, "-"))
			node = create_add(false, node, mul(env), env->token);
		else
			return (node);
	}
}

static Node *relational(Env *env)
{
	Node *node = add(env);
	for (;;)
	{
		if (consume(env, "<"))
			node = new_node(ND_LESS, node, add(env));
		else if (consume(env, "<="))
			node = new_node(ND_LESSEQ, node, add(env));
		else if (consume(env, ">"))
			node = new_node(ND_LESS, add(env), node);
		else if (consume(env, ">="))
			node = new_node(ND_LESSEQ, add(env), node);
		else
			return node;

		node->type = new_primitive_type(INT);

		if (!can_compared(node->lhs->type, node->rhs->type))
			error_at(env->token->str,
					"%sと%sを比較することはできません",
					get_type_name(node->lhs->type), get_type_name(node->rhs->type));
	}
}

static Node *equality(Env *env)
{
	Node *node = relational(env);
	for (;;)
	{
		if (consume(env, "=="))
			node = new_node(ND_EQUAL, node, relational(env));
		else if (consume(env, "!="))
			node = new_node(ND_NEQUAL, node, relational(env));
		else
			return node;

		node->type = new_primitive_type(INT);

		if (!can_compared(node->lhs->type, node->rhs->type))
			error_at(env->token->str,
					"%sと%sを比較することはできません",
					get_type_name(node->lhs->type), get_type_name(node->rhs->type));
	}
}

static Node	*conditional(Env *env)
{
	Node	*node;

	node = equality(env);
	if (consume(env, "&&"))
	{
		node = new_node(ND_COND_AND, node, conditional(env));
		node->type = new_primitive_type(INT);

		// TODO 型チェック
		//if (!is_integer_type(node->lhs)
		//|| !is_integer_type(node->rhs))
		//	error_at(env->token->str, "左辺か右辺の型が整数型ではありません (&&)");
	}
	else if (consume(env, "||"))
	{
		node = new_node(ND_COND_OR, node, conditional(env));
		node->type = new_primitive_type(INT);

		//if (!is_integer_type(node->lhs)
		//|| !is_integer_type(node->rhs))
		//	error_at(env->token->str, "左辺か右辺の型が整数型ではありません (||)");
	}
	return (node);
}

static Node	*create_assign(Node *lhs, Node *rhs, Token *tok)
{
	Node	*node;

	node = new_node(ND_ASSIGN, lhs, rhs);

	// 代入可能な型かどうか確かめる。
	if (lhs->type->ty == VOID
	|| rhs->type->ty == VOID)
		error_at(tok->str, "voidを宣言、代入できません");

	if (!type_equal(rhs->type, lhs->type))
	{
		if (type_can_cast(rhs->type, lhs->type, false))
		{
			printf("#assign (%s) <- (%s)\n",
					get_type_name(lhs->type),
					get_type_name(rhs->type));
			node->rhs = new_node(ND_CAST, rhs, NULL);
			rhs = node->rhs;
			rhs->type = lhs->type;
		}
		else
		{
			error_at(tok->str, "左辺(%s)に右辺(%s)を代入できません",
					get_type_name(lhs->type),
					get_type_name(rhs->type));
		}
	}
	node->type = lhs->type;
	return (node);
}

static Node	*assign(Env *env)
{
	Node	*node;

	node = conditional(env);
	if (consume(env, "="))
		node = create_assign(node, assign(env), env->token);
	else if (consume(env, "+="))
	{
		node = create_add(true, node, assign(env), env->token);
		node->kind = ND_COMP_ADD;
	}
	else if (consume(env, "-="))
	{
		node = create_add(false, node, assign(env), env->token);
		node->kind = ND_COMP_SUB;
	}
	else if (consume(env, "*="))
	{
		node = create_mul(0, node, assign(env), env->token);
		node->kind = ND_COMP_MUL;
	}
	else if (consume(env, "/="))
	{
		node = create_mul(1, node, assign(env), env->token);
		node->kind = ND_COMP_DIV;
	}
	else if (consume(env, "%="))
	{
		node = create_mul(2, node, assign(env), env->token);
		node->kind = ND_COMP_MOD;
	}
	return (node);
}

static Node	*expr(Env *env)
{
	return assign(env);
}

Stack	*sbstack;

SBData	*sbdata_new(bool isswitch, int start, int end)
{
	SBData	*tmp;

	tmp = (SBData *)malloc(sizeof(SBData));
	tmp->isswitch = isswitch;
	tmp->startlabel = start;
	tmp->endlabel = end;

	tmp->type = NULL;
	tmp->cases = NULL;
	tmp->defaultLabel = -1;
	return (tmp);
}

void	sb_forwhile_start(int startlabel, int endlabel)
{
	stack_push(&sbstack, sbdata_new(false, startlabel, endlabel));
}

void	sb_switch_start(Type *type, int endlabel, int defaultLabel)
{
	SBData	*tmp;

	tmp = sbdata_new(true, -1, endlabel);
	tmp->type = type;
	tmp->defaultLabel = defaultLabel;
	stack_push(&sbstack, tmp);
}

SBData	*sb_end(void)
{
	return ((SBData *)stack_pop(&sbstack));
}

SBData	*sb_peek(void)
{
	return (SBData *)stack_peek(sbstack);
}

SBData	*sb_search(bool	isswitch)
{
	Stack	*tmp;
	SBData	*data;

	tmp = sbstack;
	while (tmp != NULL)
	{
		data = (SBData *)tmp->data;
		if (data->isswitch == isswitch)
			return (data);
		tmp = tmp->prev;
	}
	return (NULL);
}

int	switchCaseCount = 0;

static int	add_switchcase(SBData *sbdata, int number)
{
	int			count;
	SwitchCase	*tmp;

	count = switchCaseCount++;

	tmp = (SwitchCase *)malloc(sizeof(SwitchCase));
	tmp->value = number;
	tmp->label = count;
	tmp->next = sbdata->cases;
	sbdata->cases = tmp;

	//TODO 被りチェック

	return (count);
}

// ifの後ろの括弧から読む
static Node	*read_ifblock(Env *env)
{
	Node	*node;

	if (!consume(env, "("))
		error_at(env->token->str, "(ではないトークンです");
	node = new_node(ND_IF, expr(env), NULL);
	if (!consume(env, ")"))
		error_at(env->token->str, ")ではないトークンです");
	node->rhs = stmt(env);
	if (consume_with_type(env, TK_ELSE))
	{
		if (consume_with_type(env, TK_IF))
			node->elsif = read_ifblock(env);
		else
			node->els = stmt(env);
	}
	return (node);
}

// TODO 条件の中身がintegerか確認する
static Node	*stmt(Env *env)
{
	Node	*node;
	Type	*type;
	Token 	*ident;

	if (consume_with_type(env, TK_RETURN))
	{
		// TODO 型チェック
		if (consume(env, ";"))
		{
			node = new_node(ND_RETURN, NULL, NULL);
			return (node);
		}
		else
			node = new_node(ND_RETURN, expr(env), NULL);
	}
	else if (consume_with_type(env, TK_IF))
	{
		return (read_ifblock(env));
	}
	else if (consume_with_type(env, TK_WHILE))
	{
		if (!consume(env, "("))
			error_at(env->token->str, "(ではないトークンです");
		node = new_node(ND_WHILE, expr(env), NULL);
		if (!consume(env, ")"))
			error_at(env->token->str, ")ではないトークンです");

		sb_forwhile_start(-1, -1);
		if (!consume(env, ";"))
			node->rhs = stmt(env);
		sb_end();

		return node;
	}
	else if (consume_with_type(env, TK_DO))
	{
		sb_forwhile_start(-1, -1);
		node = new_node(ND_DOWHILE, stmt(env), NULL);
		sb_end();

		if (!consume_with_type(env, TK_WHILE))
			error_at(env->token->str, "whileが必要です");

		if (!consume(env, "("))
			error_at(env->token->str, "(ではないトークンです");
		if (!consume(env, ";"))
			node->rhs = expr(env);
		if (!consume(env, ")"))
			error_at(env->token->str, ")ではないトークンです");
	}
	else if (consume_with_type(env, TK_FOR))
	{
		if (!consume(env, "("))
			error_at(env->token->str, "(ではないトークンです");
		node = new_node(ND_FOR, NULL, NULL);
		// for init
		if (!consume(env, ";"))
		{
			node->for_init = expr(env);
			if (!consume(env, ";"))
				error_at(env->token->str, ";が必要です");
		}
		// for if
		if (!consume(env, ";"))
		{
			node->for_if = expr(env);
			if (!consume(env, ";"))
				error_at(env->token->str, ";が必要です");
		}
		// for next
		if (!consume(env, ")"))
		{
			node->for_next = expr(env);
			if(!consume(env, ")"))
				error_at(env->token->str, ")ではないトークンです");
		}

		// stmt
		sb_forwhile_start(-1, -1);
		if (!consume(env, ";"))
			node->lhs = stmt(env);
		sb_end();

		return node;
	}
	else if (consume_with_type(env, TK_SWITCH))
	{
		if (!consume(env, "("))
			error_at(env->token->str, "(ではないトークンです");
		node = new_node(ND_SWITCH, expr(env), NULL);
		if (!consume(env, ")"))
			error_at(env->token->str, ")ではないトークンです");

		// TODO exprの型チェック, キャストも
		if (!is_integer_type(node->lhs->type))
			error_at(env->token->str, "switch文で整数型以外の型で分岐することはできません");

		sb_switch_start(node->lhs->type, -1, -1);
		node->rhs = stmt(env);
		SBData	*data = sb_end();

		node->switch_cases = data->cases;
		node->switch_has_default = data->defaultLabel != -1;
		return (node);
	}
	else if (consume_with_type(env, TK_CASE))
	{
		int	number;

		if (!consume_number(env, &number))
			error_at(env->token->str, "数値が必要です");
		if (!consume(env, ":"))
			error_at(env->token->str, ":が必要です");

		node = new_node(ND_CASE, NULL, NULL);

		SBData	*sbdata = sb_search(true);
		if (sbdata == NULL)
			error_at(env->token->str, "caseに対応するswitchがありません");
		int label = add_switchcase(sbdata, number);

		node->val = number;
		node->switch_label = label;
		return (node);
	}
	else if (consume_with_type(env, TK_BREAK))
	{
		SBData	*sbdata = sb_peek();
		if (sbdata == NULL)
			error_at(env->token->str, "breakに対応するstatementがありません");
		node = new_node(ND_BREAK, NULL, NULL);
	}
	else if (consume_with_type(env, TK_CONTINUE))
	{
		if (sb_search(false) == NULL)
			error_at(env->token->str, "continueに対応するstatementがありません");
		node = new_node(ND_CONTINUE, NULL, NULL);
	}
	else if (consume_with_type(env, TK_DEFAULT))
	{
		if (!consume(env, ":"))
			error_at(env->token->str, ":が必要です");

		SBData *data = sb_search(true);
		if (data == NULL)
			error_at(env->token->str, "defaultに対応するstatementがありません");
		if (data->defaultLabel != -1)
			error_at(env->token->str, "defaultが2個以上あります");

		data->defaultLabel = 1;
		node = new_node(ND_DEFAULT, NULL, NULL);
		return (node);
	}
	else if (consume(env, "{"))
	{
		node = new_node(ND_BLOCK, NULL, NULL);
		Node *start = node;
		while (!consume(env, "}"))
		{
			node->lhs = stmt(env);
			node->rhs = new_node(ND_BLOCK, NULL, NULL);
			node = node->rhs;
		}
		return start;
	}
	else
	{
		// 構造体なら宣言の可能性がある
		// TODO ここはfilescopeのコピペ
		if (consume_with_type(env, TK_STRUCT))
		{
			ident = consume_ident(env);
			if (ident == NULL)
				error_at(env->token->str, "構造体の識別子が必要です");

			if (consume(env, "{"))
			{
				node = read_struct_block(env, ident);
				// ;なら構造体の宣言
				// そうでないなら型宣言
				if (consume(env, ";"))
					return (node);
			}
			type = new_struct_type(env, ident->str, ident->len);
			consume_type_ptr(env, &type);
		}
		else
			type = consume_type_before(env, false);

		if (type != NULL)
		{
			ident = consume_ident(env);

			if (ident == NULL)
				error_at(env->token->str, "識別子が必要です");

			expect_type_after(env, &type);

			// TODO voidチェックは違うパスでやりたい....
			if (!is_declarable_type(type))
				error_at(env->token->str, "宣言できない型の変数です");

			LVar *created = create_local_var(env, ident->str, ident->len, type, false);

			node = new_node(ND_DEFVAR, NULL, NULL);
			node->type = type;
			node->offset = created->offset;

			// 宣言と同時に代入
			if (consume(env, "="))
			{
				node = new_node(ND_LVAR, NULL, NULL);
				node->offset = created->offset;
				node->type = created->type;
				node = create_assign(node, expr(env), env->token);
			}
		}
		else
		{
			node = expr(env);
		}
	}
	if(!consume(env, ";"))
		error_at(env->token->str, ";ではないトークン(Kind : %d , %s)です", env->token->kind, strndup(env->token->str, env->token->len));

	return node;
}

static Node	*global_var(Env *env, Type *type, Token *ident)
{
	int		i;
	Node	*node;

	// 後ろの型を読む
	expect_type_after(env, &type);

	node = new_node(ND_DEFVAR_GLOBAL, NULL, NULL);
	node->type = type;
	node->var_name = strndup(ident->str, ident->len);
	node->var_name_len = ident->len;

	// TODO Voidチェックは違うパスでやりたい....
	if (!is_declarable_type(type))
		error_at(env->token->str, "宣言できない型の変数です");
	

	// TODO 代入文 , 定数じゃないとだめ

	if (!consume(env, ";"))
		error_at(env->token->str, ";が必要です。");

	i = -1;
	while (env->global_vars[++i])
		continue;
	env->global_vars[i] = node;

	return node;
}

// {以降を読む
Node	*read_struct_block(Env *env, Token *ident)
{
	Type				*type;
	StructDef			*def;
	StructMemberElem	*tmp;
	int					i;
	int					typesize;
	int					maxsize;

	for (i = 0; env->struct_defs[i]; i++)
		continue ;

	def = calloc(1, sizeof(StructDef));
	def->name = ident->str;
	def->name_len = ident->len;
	def->mem_size = -1;
	def->members = NULL;
	env->struct_defs[i] = def;

	printf("# READ STRUCT %s\n", strndup(ident->str, ident->len));

	while (1)
	{
		if (consume(env, "}"))
			break;

		type = consume_type_before(env, false);
		if (type == NULL)
			error_at(env->token->str, "型宣言が必要です");
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です");
		expect_type_after(env, &type);
		if (!consume(env, ";"))
			error_at(env->token->str, ";が必要です");

		tmp = calloc(1, sizeof(StructMemberElem));
		tmp->name = ident->str;
		tmp->name_len = ident->len;
		tmp->type = type;
		tmp->next = def->members;

		// 型のサイズを取得
		typesize = type_size(type);
		if (typesize == -1)
			error_at(ident->str, "型のサイズが確定していません");

		maxsize = max_type_size(type);

		// offsetをoffset + typesizeに設定
		if (def->members ==  NULL)
			tmp->offset = typesize;
		else
		{
			i = def->members->offset;
			if (maxsize < 4)
			{
				if (i % 4 + typesize > 4)
					tmp->offset = ((i + 4) / 4 * 4) + typesize;
				else
					tmp->offset = i + typesize;
			}
			else if (maxsize == 4)
				tmp->offset = ((i + 3) / 4) * 4 + typesize;
			else
				tmp->offset = ((i + 7) / 8) * 8 + typesize;
		}

		printf("#  OFFSET OF %s : %d\n", strndup(ident->str, ident->len), tmp->offset);
		def->members = tmp;
	}

	// メモリサイズを決定
	if (def->members == NULL)
		def->mem_size = 0;
	else
	{
		maxsize = max_type_size(new_struct_type(env, def->name, def->name_len));
		printf("#  MAX_SIZE = %d\n", maxsize);
		def->mem_size = align_to(def->members->offset, maxsize);
	}
	printf("#  MEMSIZE = %d\n", def->mem_size);

	// offsetを修正
	for (tmp = def->members; tmp != NULL; tmp = tmp->next)
	{
		tmp->offset -= type_size(tmp->type);
	}

	return (new_node(ND_STRUCT_DEF, NULL, NULL));
}

// TODO ブロックを抜けたらlocalsを戻す
// TODO 変数名の被りチェックは別のパスで行う
// (まで読んだところから読む
static Node	*funcdef(Env *env, Type *ret_type, Token *ident)
{
	Node	*node;

	env->locals = NULL;

	// create node
	node = new_node(ND_FUNCDEF, NULL, NULL);
	node->fname = strndup(ident->str, ident->len);
	node->flen = ident->len;
	node->ret_type = ret_type;
	node->argdef_count = 0;

	// args
	if (!consume(env, ")"))
	{
		for (;;)
		{
			// 型宣言の確認
			Type *type = consume_type_before(env, false);
			if (type == NULL)
				error_at(env->token->str,"型宣言が必要です");

			// 仮引数名
			Token *arg = consume_ident(env);
			if (arg == NULL)
			{
				// voidなら引数0個
				if (type->ty == VOID && node->argdef_count == 0)
				{
					if (!consume(env, ")"))
						error_at(env->token->str, ")が見つかりませんでした。");
					break ;
				}
				error_at(env->token->str, "仮引数が必要です");
			}
			// LVarを作成
			LVar *created = create_local_var(env, arg->str, arg->len, type, true);
			// arrayを読む
			expect_type_after(env, &type);

			// TODO Voidチェックは違うパスでやりたい....
			if (!is_declarable_type(type))
				error_at(env->token->str, "宣言できない型の変数です");

			// 型情報を保存
			type->next = node->arg_type;
			node->arg_type = type;
			node->argdef_count++;
			
			// )か,
			if (consume(env, ")"))
				break;
			if (!consume(env, ","))
				error_at(env->token->str, ",が必要です");
		}
	}
	else
	{
		// 可変長
		node->argdef_count = -1;
	}

	printf("# READ ARGS\n");

	// func_defsに代入
	// TODO 関数名被り
	// TODO プロトタイプ宣言後の関数定義
	if (consume(env, ";"))
	{
		node->kind = ND_PROTOTYPE;

		int i = 0;
		while (env->func_protos[i])
			i += 1;
		env->func_protos[i] = node;
		node->locals = env->locals;
	}
	else
	{
		int i = 0;
		while (env->func_defs[i])
			i += 1;
		env->func_defs[i] = node;
		node->locals = env->locals;
		node->lhs = stmt(env);
		node->locals = env->locals;
	}
	
	printf("# CREATED FUNC %s\n", strndup(node->fname, node->flen));

	return node;
}

static Node	*read_typedef(Env *env)
{
	Type		*type;
	Token		*token;
	TypedefPair	*pair;

	// 型を読む
	type = consume_type_before(env, true);
	if (type == NULL)
		error_at(env->token->str, "型宣言が必要です");
	expect_type_after(env, &type);

	// 識別子を読む
	token = consume_ident(env);
	if (token == NULL)
		error_at(env->token->str, "識別子が必要です");

	// ペアを追加
	pair = malloc(sizeof(TypedefPair));
	pair->name = token->str;
	pair->name_len = token->len;
	pair->type = type;
	linked_list_insert(env->type_alias, pair);

	if (!consume(env, ";"))
		error_at(env->token->str, ";が必要です");

	return (new_node(ND_TYPEDEF, NULL, NULL));
}

static Node	*filescope(Env *env)
{
	Token	*ident;
	Type	*ret_type;
	bool	is_static;
	Node	*node;

	// typedef
	// TODO 後でTK_TYPEDEFにする
	if (consume_ident_str(env, "typedef"))
	{
		return (read_typedef(env));
	}

	is_static = false;
	if (consume_ident_str(env, "static"))
	{
		is_static = true;
	}
	
	// structの宣言か返り値がstructか
	if (consume_with_type(env, TK_STRUCT))
	{
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "構造体の識別子が必要です");
			
		if (consume(env, "{"))
		{
			node = read_struct_block(env, ident);
			// ;なら構造体の宣言
			// そうでないなら返り値かグローバル変数
			if (consume(env, ";"))
				return (node);
		}
		ret_type = new_struct_type(env, ident->str, ident->len);
		consume_type_ptr(env, &ret_type);
	}
	else
		ret_type = consume_type_before(env, true);

	// TODO 一旦staticは無視
	// グローバル変数か関数宣言か
	if (ret_type != NULL)
	{
		// ident
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "不明なトークンです");

		// function definition
		if (consume(env, "("))
			return funcdef(env, ret_type, ident);
		else
			return global_var(env, ret_type, ident);
	}

	error_at(env->token->str, "構文解析に失敗しました[filescope kind:%d]", env->token->kind);
	return (NULL);
}

void	program(Env *env)
{
	int	i;

	i = 0;
	while (!at_eof(env))
		env->code[i++] = filescope(env);
	env->code[i] = NULL;
}

int	typealias_search(void *a, void *t)
{
	TypedefPair	*pair;
	char		*target;

	pair = (TypedefPair *)a;
	target = (char *)t;
	if (pair->name_len != strlen(target))
		return (-1);
	return (strncmp(pair->name, target, pair->name_len));
}

Env	*parse(Token *tok)
{
	Env	*env;

	env = calloc(1, sizeof(Env));
	env->token = tok;
	env->type_alias = linked_list_new(typealias_search);
	program(env);
	return env;
}
