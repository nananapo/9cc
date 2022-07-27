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

LVar		*find_lvar(Env *env, char *str, int len);
FindEnumRes	*find_enum(Env *env, char *str, int len);
LVar		*create_local_var(Env *env, char *name, int len, Type *type, bool is_arg);
Type	*type_cast_forarg(Type *type);
LVar	*copy_lvar(LVar *f);
void	alloc_argument_simu(LVar *first, LVar *lvar);

Node	*get_function_by_name(Env *env, char *name, int len);
Node	*new_node(NodeKind kind, Node *lhs, Node *rhs);
Node	*new_node_num(int val);
bool	consume_enum_key(Env *env, Type **type, int *value);
bool	consume_charlit(Env *env, int *number);
int		get_str_literal_index(Env *env, char *str, int len);
Node	 *cast(Env *env, Node *node, Type *to);
Node	*call(Env *env, Token *tok);
Node	*read_suffix_increment(Env *env, Node *node);
Node	*read_deref_index(Env *env, Node *node);
Node	*primary(Env *env);
Node	*arrow_loop(Env *env, Node *node);
Node	*arrow(Env *env);
Node	*unary(Env *env);
Node	*create_mul(int type, Node *lhs, Node *rhs, Token *tok);
Node	*mul(Env *env);
Node	*create_add(bool isadd, Node *lhs, Node *rhs, Token *tok);
Node	*add(Env *env);
Node	*shift(Env *env);
Node	*relational(Env *env);
Node	*equality(Env *env);
Node	*bitwise_and(Env *env);
Node	*bitwise_or(Env *env);
Node	*bitwise_xor(Env *env);
Node	*conditional_and(Env *env);
Node	*conditional_or(Env *env);
Node	*conditional_op(Env *env);
Node	*create_assign(Env *env, Node *lhs, Node *rhs, Token *tok);
Node	*assign(Env *env);
Node	*expr(Env *env);
SBData	*sbdata_new(bool isswitch, int start, int end);
void	sb_forwhile_start(int startlabel, int endlabel);
void	sb_switch_start(Type *type, int endlabel, int defaultLabel);
SBData	*sb_end(void);
SBData	*sb_peek(void);
SBData	*sb_search(bool	isswitch);
int		add_switchcase(SBData *sbdata, int number);
Node	*read_ifblock(Env *env);
Node	*stmt(Env *env);
Node	*expect_constant(Env *env, Type *type);
Node	*global_var(Env *env, Type *type, Token *ident, bool is_extern, bool is_static);
Node	*read_struct_block(Env *env, Token *ident);
Node	*read_enum_block(Env *env, Token *ident);
Node	*read_union_block(Env *env, Token *ident);
Node	*funcdef(Env *env, Type *type, Token *ident, bool is_static);
Node	*read_typedef(Env *env);
Node	*filescope(Env *env);
void	program(Env *env);
Env		*parse(Token *tok);

int		switchCaseCount = 0;
Stack	*sbstack;

static int max(int a, int b)
{
	if (a < b)
		return (b);
	return (a);
}

Node	*get_function_by_name(Env *env, char *name, int len)
{
	int		i;
	Node	*tmp;

	i = 0;
	while (env->func_defs[i])
	{
		tmp = env->func_defs[i];
		if (tmp->flen == len && strncmp(tmp->fname, name, len) == 0)
			return tmp;
		i++;
	}

	i = 0;
	while (env->func_protos[i])
	{
		tmp = env->func_protos[i];
		if (tmp->flen == len && strncmp(tmp->fname, name, len) == 0)
			return tmp;
		i++;
	}
	return NULL;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node *node;

	node = calloc(1, sizeof(Node));
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

	// TODO とりあえずこれでprint_global_constantのchar*を区別
	node->str_index = -1;

	return node;
}

Node	*new_node_num(int val)
{
	Node *node;

	node = new_node(ND_NUM, NULL, NULL);
	node->val = val;

	//if (val < 127 && val > -128)
	//	node->type = new_primitive_type(TY_CHAR);
	//else
		node->type = new_primitive_type(TY_INT);
	return node;
}











bool	consume_enum_key(Env *env, Type **type, int *value)
{
	Token		*tok;
	FindEnumRes	*fer;

	if (env->token->kind != TK_IDENT)
		return (false);
	tok = env->token;
	fer = find_enum(env, tok->str, tok->len);
	if (fer != NULL)
	{
		consume_ident(env);
		if (type != NULL)
			*type = fer->type;
		if (value != NULL)
			*value = fer->value;
		return (true);
	}
	return (false);
}

bool consume_charlit(Env *env, int *number)
{
	if (env->token->kind != TK_CHAR_LITERAL)
		return (false);

	*number = get_char_to_int(env->token->str, env->token->strlen_actual);
	if (*number == -1)
		error_at(env->token->str, "不明なエスケープシーケンスです");

	env->token = env->token->next; // 進める
	return (true);
}








// リテラルを探す
int	get_str_literal_index(Env *env, char *str, int len)
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

Node *cast(Env *env, Node *node, Type *to)
{
	node = new_node(ND_CAST, node, NULL);
	if (!type_can_cast(node->lhs->type, to, true))
		error_at(env->token->str, "%sを%sにキャストできません",
					get_type_name(node->lhs->type), get_type_name(to));
	node->type = to;
	return (node);
}

Node *call(Env *env, Token *tok)
{
	Node	*node;
	Node	*args;
	Node	*readarg;
	Node	*ntmp;
	Node	*refunc;

	debug("# CALL %s START\n", strndup(tok->str, tok->len));

	node  = new_node(ND_CALL, NULL, NULL);
	node->fname = tok->str;
	node->flen = tok->len;
	node->args = NULL;
	node->argdef_count = 0;

	args = NULL;

	if (!consume(env, ")"))
	{
		readarg = expr(env);
		args = readarg;
		node->argdef_count = 1;

		for (;;)
		{	
			if (consume(env, ")"))
				break;
			if (!consume(env, ","))
				error_at(env->token->str, "トークンが,ではありません");
			
			readarg = expr(env);
			readarg->type = readarg->type; // 配列からポインタにする
			readarg->next = NULL;
			if (args == NULL)
				args = readarg;
			else
			{
				for (ntmp = args; ntmp; ntmp = ntmp->next)
				{
					if (ntmp->next == NULL)
					{
						ntmp->next = readarg;
						break ;
					}
				}
			}
			node->argdef_count += 1;
		}
	}

	refunc = get_function_by_name(env, node->fname, node->flen);
	// 関数定義が見つからない場合エラー
	if (refunc == NULL)
		error_at(env->token->str, "warning : 関数%sがみつかりません\n", strndup(node->fname, node->flen));

	// 引数の数を確認
	if (refunc->argdef_count != -1
		&& ((!refunc->is_variable_argument && node->argdef_count != refunc->argdef_count)
			|| (refunc->is_variable_argument && node->argdef_count < refunc->argdef_count)))
		error_at(env->token->str, "関数%sの引数の数が一致しません\n expected : %d\n actual : %d", strndup(node->fname, node->flen), refunc->argdef_count, node->argdef_count);

	// 関数の情報を保存しておく
	node->is_variable_argument = refunc->is_variable_argument;

	// 引数を定義と比較
	LVar	*def;
	LVar	*lastdef;
	LVar	*firstdef;
	LVar	*lvtmp;
	int		i;
	int		j;

	if (refunc->locals != NULL)
	{
		// 返り値がMEMORYなら二個進める
		if (is_memory_type(refunc->ret_type))
		{
			def = refunc->locals->next->next;
			if (def != NULL)
				def = copy_lvar(def);
		}
		else
			def = copy_lvar(refunc->locals);
	}
	else
		def = NULL;
	firstdef = def;
	lastdef = NULL;

	for (i = 0; i < node->argdef_count; i++)
	{
		debug("#  READ ARG(%d) START\n", i);

		if (def != NULL && def->is_arg)
		{
			debug("#  is ARG\n");
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

				args = cast(env, args, def->type);
				args->next = args->lhs->next;
			}
		}
		else
		{
			// defがNULL -> 可変長引数
			debug("#  is VA\n");

			// create_local_varからのコピペ
			def = calloc(1, sizeof(LVar));
			def->name = "VA";
			def->len = 2;
			def->type = type_cast_forarg(args->type);
			def->is_arg = true;
			def->arg_regindex = -1;
			def->next = NULL;
			def->is_dummy = false;

			alloc_argument_simu(firstdef, def); 

			debug("#ASSIGNED %d\n", def->arg_regindex);

			lastdef->next = def;
		}

		// use->localsにdefを入れる
		args->locals = def;

		// 格納
		if (node->args == NULL)
			node->args = args;
		else
		{
			ntmp = node->args;
			for (j = 0; j < i - 1; j++)
				ntmp = ntmp->next;
			ntmp->next = args;
		}

		// 進める
		lastdef = def;
		if (def->next != NULL)
		{
			lvtmp = copy_lvar(def->next);
			def->next = lvtmp;
			def = lvtmp;
		}
		else
			def = NULL;
		args = args->next;

		debug("#  READ ARG(%d) END\n", i);
	}

	// 型を返り値の型に設定
	node->type = refunc->ret_type;

	// 返り値がMEMORYかstructなら、それを保存する用の場所を確保する
	if (is_memory_type(refunc->ret_type)
		|| refunc->ret_type->ty == TY_STRUCT)
	{
		node->call_mem_stack = create_local_var(env, "", 0, refunc->ret_type, false);
		node->call_mem_stack->is_dummy = true;
	}

	debug("# CALL END\n");

	return node;
}

// 後置インクリメント, デクリメント
Node	*read_suffix_increment(Env *env, Node *node)
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
// TODO エラーメッセージが足し算用になってしまう
Node	*read_deref_index(Env *env, Node *node)
{
	Node	*add;

	while (consume(env, "["))
	{
		add = create_add(true, node, expr(env), env->token);
		node = new_node(ND_DEREF, add, NULL);
		node->type = node->lhs->type->ptr_to;

		if (!consume(env, "]"))
			error_at(env->token->str, "%s");
	}
	return read_suffix_increment(env, node);
}

Node *primary(Env *env)
{
	Token	*tok;
	Node	*node;
	Type	*type_cast;
	int 	number;

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
		node = cast(env, unary(env), type_cast);
		return read_deref_index(env, node);
	}

	// enumの値
	Type	*enum_type;
	int		enum_value;

	if (consume_enum_key(env, &enum_type, &enum_value))
	{
		node = new_node_num(enum_value);
		node->type = enum_type;
		return read_deref_index(env, node);
	}

	// identかcall
	LVar	*lvar;
	Node	*glovar;

	tok = consume_ident(env);
	if (tok)
	{
		// call func
		if (consume(env, "("))
		{
			node = call(env, tok);
			return read_deref_index(env, node);
		}

		// ローカル変数
		lvar = find_lvar(env, tok->str, tok->len);
		if (lvar != NULL)
		{
			node = new_node(ND_LVAR, NULL, NULL);
			node->lvar = lvar;
			node->type = lvar->type;
			return read_deref_index(env, node);
		}

		// グローバル変数
		glovar = find_global(env, tok->str, tok->len);
		if (glovar != NULL)
		{
			node = new_node(ND_LVAR_GLOBAL, NULL, NULL);
			node->var_name = glovar->var_name;
			node->var_name_len = glovar->var_name_len;
			node->type = glovar->type;
			return read_deref_index(env, node);
		}
		error_at(tok->str,"%sが定義されていません", strndup(tok->str, tok->len));
	}

	// string
	tok = consume_str_literal(env);
	if (tok)
	{
		node = new_node(ND_STR_LITERAL, NULL, NULL);
		node->str_index = get_str_literal_index(env, tok->str, tok->len);
		node->type = new_type_ptr_to(new_primitive_type(TY_CHAR));
		return read_deref_index(env, node);
	}

	// char
	tok = consume_char_literal(env);
	if (tok)
	{
		number = get_char_to_int(tok->str, tok->strlen_actual);
		if (number == -1)
			error_at(tok->str, "不明なエスケープシーケンスです");
		node = new_node_num(number);
		node->type = new_primitive_type(TY_CHAR);
		return read_deref_index(env, node); // charの後ろに[]はおかしいけれど、とりあえず許容
	}

	// 数
	if (!consume_number(env, &number))
		error_at(env->token->str, "数字が必要です");
	node = new_node_num(number);

	return read_deref_index(env, node);
}


Node	*arrow_loop(Env *env, Node *node)
{
	Token		*ident;
	MemberElem	*elem;
	Type		*type;

	if (consume(env, "->"))
	{
		type = node->type;

		if (!can_use_arrow(type))
			error_at(env->token->str, "%sに->を適用できません", get_type_name(type));
		
		ident = consume_ident(env);

		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です\n (arrow_loop)");

		elem = get_member_by_name(type->ptr_to, ident->str, ident->len);
		if (elem == NULL)
			error_at(env->token->str, "識別子が存在しません", strndup(ident->str, ident->len));
		
		node = new_node(ND_MEMBER_PTR_VALUE, node, NULL);
		node->elem = elem;
		node->type = elem->type;

		node = read_deref_index(env, node);

		return arrow_loop(env, node);
	}
	else if (consume(env, "."))
	{
		type = node->type;

		if (!can_use_dot(type))
			error_at(env->token->str, "%sに.を適用できません", get_type_name(type));

		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です\n (arrow_loop)");

		elem = get_member_by_name(type, ident->str, ident->len);
		if (elem == NULL)
			error_at(env->token->str, "識別子が存在しません", strndup(ident->str, ident->len));

		node = new_node(ND_MEMBER_VALUE, node, NULL);
		node->elem = elem;
		node->type = elem->type;

		node = read_deref_index(env, node);

		return arrow_loop(env, node);
	}
	else
		return (node);
}

Node	*arrow(Env *env)
{
	Node	*node;

	node = primary(env);
	return (arrow_loop(env, node));
}

Node *unary(Env *env)
{
	Node	*node;
	Type	*type;

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
		&& node->lhs->kind != ND_MEMBER_VALUE
		&& node->lhs->kind != ND_MEMBER_PTR_VALUE
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
		node->type = new_primitive_type(TY_INT);
		return (node);
	}
	else if (consume(env, "~"))
	{
		node = new_node(ND_BITWISE_NOT, unary(env), NULL);
		node->type = node->lhs->type;

		if (!is_integer_type(node->lhs->type))
			error_at(env->token->str, "整数ではない型に~を適用できません");
		return (node);
	}
	else if (consume_with_type(env, TK_SIZEOF))
	{
		// とりあえずsizeof (型)を読めるように
		if (consume(env, "("))
		{
			type = consume_type_before(env, true);
			if (type == NULL)
			{
				node = unary(env);
				node = new_node_num(get_type_size(node->type));
			}
			else
				node = new_node_num(get_type_size(type));
		
			if (!consume(env, ")"))
				error_at(env->token->str, ")が必要です");
			return (node);
		}
		else
		{
			node = unary(env);
			node = new_node_num(get_type_size(node->type));
			return (node);
		}
	}

	return arrow(env);
}


/*
 * type:
 * 0 : *
 * 1 : /
 * 2 : %
 */
Node	*create_mul(int type, Node *lhs, Node *rhs, Token *tok)
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

	node->type = new_primitive_type(TY_INT);
	return (node);
}

Node *mul(Env *env)
{
	Node *node;

	node = unary(env);
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

Node	*create_add(bool isadd, Node *lhs, Node *rhs, Token *tok)
{
	Node	*node;
	Type	*l;
	Type	*r;
	int		size;

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
		node->type = new_primitive_type(TY_INT);// TODO size_tにする

		size = get_type_size(l->ptr_to);
		if (size == 0 || size > 1)
		{
			if (size == 0)
				fprintf(stderr, "WARNING : サイズ0の型のポインタ型どうしの加減算は未定義動作です");
			node = new_node(ND_DIV, node, new_node_num(size));
			node->type = new_primitive_type(TY_INT);
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
		if (r->ty == TY_INT)
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

Node	*add(Env *env)
{
	Node	*node;

	node = mul(env);
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

// TODO 型チェック
Node	*shift(Env *env)
{
	Node	*node;

	node = add(env);
	if (consume(env, "<<"))
	{
		node = new_node(ND_SHIFT_LEFT, node, shift(env));
		
		if (!is_integer_type(node->lhs->type)
			|| !is_integer_type(node->rhs->type))
			error_at(env->token->str, "整数型ではない型にシフト演算子を適用できません");

		// 左辺の型にする
		node->type = node->lhs->type;
	}
	else if (consume(env, ">>"))
	{
		node = new_node(ND_SHIFT_RIGHT, node, shift(env));
		
		if (!is_integer_type(node->lhs->type)
			|| !is_integer_type(node->rhs->type))
			error_at(env->token->str, "整数型ではない型にシフト演算子を適用できません");

		// 左辺の型にする
		node->type = node->lhs->type;
	}
	return (node);
}

Node	*relational(Env *env)
{
	Node	*node;
	Type	*l_to;
	Type	*r_to;

	node = shift(env);
	for (;;)
	{
		if (consume(env, "<"))
			node = new_node(ND_LESS, node, shift(env));
		else if (consume(env, "<="))
			node = new_node(ND_LESSEQ, node, shift(env));
		else if (consume(env, ">"))
			node = new_node(ND_LESS, shift(env), node);
		else if (consume(env, ">="))
			node = new_node(ND_LESSEQ, shift(env), node);
		else
			return node;

		node->type = new_primitive_type(TY_INT);

		// 比較できるか確認
		if (!can_compared(node->lhs->type, node->rhs->type, &l_to, &r_to))
			error_at(env->token->str,"%sと%sを比較することはできません",
					get_type_name(node->lhs->type), get_type_name(node->rhs->type));
		// キャストする必要があるならキャスト
		if (!type_equal(node->lhs->type, l_to))
			node->lhs = cast(env, node->lhs, l_to);
		if (!type_equal(node->rhs->type, r_to))
			node->rhs = cast(env, node->rhs, r_to);
	}
}

Node *equality(Env *env)
{
	Node	*node;
	Type	*l_to;
	Type	*r_to;

	node = relational(env);
	for (;;)
	{
		if (consume(env, "=="))
			node = new_node(ND_EQUAL, node, relational(env));
		else if (consume(env, "!="))
			node = new_node(ND_NEQUAL, node, relational(env));
		else
			return node;

		node->type = new_primitive_type(TY_INT);

		// 比較できるか確認
		if (!can_compared(node->lhs->type, node->rhs->type, &l_to, &r_to))
			error_at(env->token->str,"%sと%sを比較することはできません",
					get_type_name(node->lhs->type), get_type_name(node->rhs->type));
		// キャストする必要があるならキャスト
		if (!type_equal(node->lhs->type, l_to))
			node->lhs = cast(env, node->lhs, l_to);
		if (!type_equal(node->rhs->type, r_to))
			node->rhs = cast(env, node->rhs, r_to);
	}
}

Node	*bitwise_and(Env *env)
{
	Node	*node;

	node = equality(env);
	if (consume(env, "&"))
	{
		node = new_node(ND_BITWISE_AND, node, bitwise_and(env));

		if (!is_integer_type(node->lhs->type)
			|| !is_integer_type(node->rhs->type))
			error_at(env->token->str, "整数型ではない型に&を適用できません");

		// TODO キャストしてから可能か考える
		if (!type_equal(node->rhs->type, node->lhs->type))
			error_at(env->token->str, "&の両辺の型が一致しません");

		node->type = node->rhs->type;
	}
	return (node);
}


Node	*bitwise_xor(Env *env)
{
	Node	*node;

	node = bitwise_and(env);
	if (consume(env, "^"))
	{
		node = new_node(ND_BITWISE_XOR, node, bitwise_xor(env));

		if (!is_integer_type(node->lhs->type)
			|| !is_integer_type(node->rhs->type))
			error_at(env->token->str, "整数型ではない型に^を適用できません");

		// TODO キャストしてから可能か考える
		if (!type_equal(node->rhs->type, node->lhs->type))
			error_at(env->token->str, "^の両辺の型が一致しません");

		node->type = node->rhs->type;
	}
	return (node);
}

// TODO 型チェック
Node	*bitwise_or(Env *env)
{
	Node	*node;

	node = bitwise_xor(env);
	if (consume(env, "|"))
	{
		node = new_node(ND_BITWISE_OR, node, bitwise_or(env));

		if (!is_integer_type(node->lhs->type)
			|| !is_integer_type(node->rhs->type))
			error_at(env->token->str, "整数型ではない型に|を適用できません");

		// TODO キャストしてから可能か考える
		if (!type_equal(node->rhs->type, node->lhs->type))
			error_at(env->token->str, "|の両辺の型が一致しません");

		node->type = node->rhs->type;
	}
	return (node);
}

// TODO 型チェック
Node	*conditional_and(Env *env)
{
	Node	*node;

	node = bitwise_or(env);
	if (consume(env, "&&"))
	{
		node = new_node(ND_COND_AND, node, conditional_and(env));
		node->type = new_primitive_type(TY_INT);
	}
	return (node);
}

// TODO 型チェック
Node	*conditional_or(Env *env)
{
	Node	*node;

	node = conditional_and(env);
	if (consume(env, "||"))
	{
		node = new_node(ND_COND_OR, node, conditional_or(env));
		node->type = new_primitive_type(TY_INT);
	}
	return (node);
}

Node	*conditional_op(Env *env)
{
	Node	*node;

	node = conditional_or(env);
	if (consume(env, "?"))
	{
		node = new_node(ND_IF, node, conditional_op(env));
		if (!consume(env, ":"))
			error_at(env->token->str, ":が必要です");
		node->els = conditional_op(env);
		if (!type_equal(node->rhs->type, node->els->type))
			error_at(env->token->str, "条件演算子の型が一致しません");
		node->type = node->rhs->type;
	}
	return (node);
}


Node	*create_assign(Env *env, Node *lhs, Node *rhs, Token *tok)
{
	Node	*node;

	node = new_node(ND_ASSIGN, lhs, rhs);

	// 代入可能な型かどうか確かめる。
	if (lhs->type->ty == TY_VOID
	|| rhs->type->ty == TY_VOID)
		error_at(tok->str, "voidを宣言、代入できません");

	if (!type_equal(rhs->type, lhs->type))
	{
		if (type_can_cast(rhs->type, lhs->type, false))
		{
			debug("#assign (%s) <- (%s)\n",
					get_type_name(lhs->type),
					get_type_name(rhs->type));
			node->rhs = cast(env, rhs, lhs->type);
			rhs = node->rhs;
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

Node	*assign(Env *env)
{
	Node	*node;

	node = conditional_op(env);
	if (consume(env, "="))
		node = create_assign(env, node, assign(env), env->token);
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

Node	*expr(Env *env)
{
	return assign(env);
}

SBData	*sbdata_new(bool isswitch, int start, int end)
{
	SBData	*tmp;

	tmp = (SBData *)calloc(1, sizeof(SBData));
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
	SBData	*result;

	result = stack_pop(&sbstack);
	return (result);
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

int	add_switchcase(SBData *sbdata, int number)
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
Node	*read_ifblock(Env *env)
{
	Node	*node;

	debug("#  IF START\n");

	if (!consume(env, "("))
		error_at(env->token->str, "(ではないトークンです");
	node = new_node(ND_IF, expr(env), NULL);

	debug("#   IF READed EXPR\n");

	if (!consume(env, ")"))
		error_at(env->token->str, ")ではないトークンです");
	node->rhs = stmt(env);

	debug("#   IF READ STMT\n");

	if (consume_with_type(env, TK_ELSE))
	{
		if (consume_with_type(env, TK_IF))
			node->elsif = read_ifblock(env);
		else
			node->els = stmt(env);
	}
	debug("#  IF END\n");
	return (node);
}

// TODO 条件の中身がintegerか確認する
Node	*stmt(Env *env)
{
	Node	*node;
	Type	*type;
	Token 	*ident;
	int		number;
	int		label;
	SBData	*data;
	Node	*start;
	LVar	*created;

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
		node = read_ifblock(env);
		return (node);
	}
	else if (consume_with_type(env, TK_WHILE))
	{
		debug("#  WHILE START\n");

		if (!consume(env, "("))
			error_at(env->token->str, "(ではないトークンです");
		node = new_node(ND_WHILE, expr(env), NULL);
		if (!consume(env, ")"))
			error_at(env->token->str, ")ではないトークンです");

		sb_forwhile_start(-1, -1);
		if (!consume(env, ";"))
			node->rhs = stmt(env);
		sb_end();

		debug("#  WHILE END\n");
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
		debug("#  FOR START\n");

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

		debug("#  FOR END\n");
		return (node);
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
		data = sb_end();

		node->switch_cases = data->cases;
		node->switch_has_default = data->defaultLabel != -1;
		return (node);
	}
	else if (consume_with_type(env, TK_CASE))
	{
		if (!consume_number(env, &number))
		{
			if (!consume_enum_key(env, NULL, &number))
			{
				if (!consume_charlit(env, &number))
					error_at(env->token->str, "定数が必要です");
			}
		}
		if (!consume(env, ":"))
			error_at(env->token->str, ":が必要です");

		node = new_node(ND_CASE, NULL, NULL);

		data = sb_search(true);
		if (data == NULL)
			error_at(env->token->str, "caseに対応するswitchがありません");
		label = add_switchcase(data, number);

		node->val = number;
		node->switch_label = label;
		return (node);
	}
	else if (consume_with_type(env, TK_BREAK))
	{
		data = sb_peek();
		if (data == NULL)
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

		data = sb_search(true);
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
		start = node;

		while (!consume(env, "}"))
		{
			debug("# START READ BLOCK LINE\n");
			node->lhs = stmt(env);
			node->rhs = new_node(ND_BLOCK, NULL, NULL);
			node = node->rhs;
			debug("# END READ BLOCK LINE\n");
		}
		return (start);
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
		// enumの可能性
		else if (consume_with_type(env, TK_ENUM))
		{
			ident = consume_ident(env);
			if (ident == NULL)
				error_at(env->token->str, "enumの識別子が必要です");

			if (consume(env, "{"))
			{
				node = read_enum_block(env, ident);
				// ;ならenumの宣言
				// そうでないなら型宣言
				if (consume(env, ";"))
					return (node);
			}
			type = new_enum_type(env, ident->str, ident->len);
			consume_type_ptr(env, &type);
		}
		// unionの可能性
		else if (consume_with_type(env, TK_UNION))
		{
			ident = consume_ident(env);
			if (ident == NULL)
				error_at(env->token->str, "unionの識別子が必要です");

			if (consume(env, "{"))
			{
				node = read_union_block(env, ident);
				// ;ならunionの宣言
				// そうでないなら型宣言
				if (consume(env, ";"))
					return (node);
			}
			type = new_union_type(env, ident->str, ident->len);
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

			created = create_local_var(env, ident->str, ident->len, type, false);

			node = new_node(ND_DEFVAR, NULL, NULL);
			node->type = type;
			node->lvar = created;

			// 宣言と同時に代入
			if (consume(env, "="))
			{
				node->kind = ND_LVAR;
				node = create_assign(env, node, expr(env), env->token);
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

Node	*expect_constant(Env *env, Type *type)
{
	Node	*node;
	Node	**next;
	int		number;
	Token	*tok;

	if (is_integer_type(type) && consume_number(env, &number))
	{
		node = new_node_num(number);
	}
	else if (type_equal(type, new_type_ptr_to(new_primitive_type(TY_CHAR)))
			&& (tok = consume_str_literal(env)) != NULL)
	{
		node = new_node(ND_STR_LITERAL, NULL, NULL);
		node->str_index = get_str_literal_index(env, tok->str, tok->len);
		node->type = new_type_ptr_to(new_primitive_type(TY_CHAR));
	}
	else if (type_equal(type, new_primitive_type(TY_CHAR)) 
			&& (tok = consume_char_literal(env)) != NULL)
	{
		number = get_char_to_int(tok->str, tok->strlen_actual);
		if (number == -1)
			error_at(tok->str, "不明なエスケープシーケンスです");
		node = new_node_num(number);
	}
	else if (is_pointer_type(type) && consume(env, "{"))
	{
		node = NULL;
		next = &node;
		for (;;)
		{
			*next = expect_constant(env, type->ptr_to);
			next = &(*next)->next;
			if (consume(env, "}"))
				break ;
			if (!consume(env, ","))
				error_at(env->token->str, ",が必要です");
		}
	}
	else
	{
		node = NULL;
		error_at(env->token->str, "定数が必要です");
	}
	return (node);
}

Node	*global_var(Env *env, Type *type, Token *ident, bool is_extern, bool is_static)
{
	int		i;
	Node	*node;

	// 後ろの型を読む
	// TODO a[]とかでも許容したい
	expect_type_after(env, &type);

	node = new_node(ND_DEFVAR_GLOBAL, NULL, NULL);
	node->type = type;
	node->var_name = strndup(ident->str, ident->len);
	node->var_name_len = ident->len;
	node->is_extern = is_extern;
	node->is_static = is_static;

	if (is_static && is_extern)
		error_at(env->token->str, "staticとexternは併用できません");

	// TODO Voidチェックは違うパスでやりたい....
	if (!is_declarable_type(type))
		error_at(env->token->str, "宣言できない型の変数です");

	// 保存
	i = -1;
	while (env->global_vars[++i])
		continue;
	env->global_vars[i] = node;

	// 代入を読む
	node->global_assign = NULL;
	if (!is_extern && consume(env, "="))
	{
		node->global_assign = expect_constant(env, node->type);
	}

	if (!consume(env, ";"))
		error_at(env->token->str, ";が必要です。");

	return node;
}

// {以降を読む
Node	*read_struct_block(Env *env, Token *ident)
{
	Type		*type;
	StructDef	*def;
	MemberElem	*tmp;
	int			typesize;
	int			maxsize;
	int			i;

	def = calloc(1, sizeof(StructDef));
	def->name = ident->str;
	def->name_len = ident->len;
	def->mem_size = -1;
	def->members = NULL;

	// 保存
	for (i = 0; env->struct_defs[i]; i++)
		continue ;
	env->struct_defs[i] = def;

	debug("# READ STRUCT %s\n", strndup(ident->str, ident->len));

	while (1)
	{
		if (consume(env, "}"))
			break;

		type = consume_type_before(env, true);
		if (type == NULL)
			error_at(env->token->str, "型宣言が必要です\n (read_struct_block)");
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です\n (read_struct_block)");
		expect_type_after(env, &type);
		if (!consume(env, ";"))
			error_at(env->token->str, ";が必要です");

		tmp = calloc(1, sizeof(MemberElem));
		tmp->name = ident->str;
		tmp->name_len = ident->len;
		tmp->type = type;
		tmp->next = def->members;

		// 型のサイズを取得
		typesize = get_type_size(type);
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

		debug("#  OFFSET OF %s : %d\n", strndup(ident->str, ident->len), tmp->offset);
		def->members = tmp;
	}

	// メモリサイズを決定
	if (def->members == NULL)
		def->mem_size = 0;
	else
	{
		maxsize = max_type_size(new_struct_type(env, def->name, def->name_len));
		debug("#  MAX_SIZE = %d\n", maxsize);
		def->mem_size = align_to(def->members->offset, maxsize);
	}
	debug("#  MEMSIZE = %d\n", def->mem_size);

	// offsetを修正
	for (tmp = def->members; tmp != NULL; tmp = tmp->next)
	{
		tmp->offset -= get_type_size(tmp->type);
	}

	return (new_node(ND_STRUCT_DEF, NULL, NULL));
}

// {以降を読む
Node	*read_enum_block(Env *env, Token *ident)
{
	int		i;
	EnumDef	*def;

	def = calloc(1, sizeof(EnumDef));
	def->name = ident->str;
	def->name_len = ident->len;
	def->kind_len = 0;

	// 保存
	for (i = 0; env->enum_defs[i]; i++)
		continue ;
	env->enum_defs[i] = def;

	debug("# READ ENUM %s\n", strndup(ident->str, ident->len));

	while (1)
	{
		if (consume(env, "}"))
			break ;
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "識別子が見つかりません");
		def->kinds[def->kind_len++] = strndup(ident->str, ident->len);
		if (!consume(env, ","))
		{
			if (!consume(env, "}"))
				error_at(env->token->str, "}が見つかりません");
			break ;
		}
		debug("# ENUM %s\n", def->kinds[def->kind_len - 1]);
	}
	return (new_node(ND_ENUM_DEF, NULL, NULL));
}

// {以降を読む
Node	*read_union_block(Env *env, Token *ident)
{
	UnionDef	*def;
	MemberElem	*tmp;
	Type		*type;
	int			i;
	int			typesize;

	def = calloc(1, sizeof(UnionDef));
	def->name = ident->str;
	def->name_len = ident->len;
	def->mem_size = 0;
	def->members = NULL;

	// 保存
	for (i = 0; env->union_defs[i]; i++)
		continue ;
	env->union_defs[i] = def;

	debug("# READ UNION %s\n", strndup(ident->str, ident->len));

	// 要素を追加 & 最大のサイズを取得
	while (1)
	{
		if (consume(env, "}"))
			break;

		type = consume_type_before(env, true);
		if (type == NULL)
			error_at(env->token->str, "型宣言が必要です\n (read_union_block)");
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です\n (read_union_block)");
		expect_type_after(env, &type);
		if (!consume(env, ";"))
			error_at(env->token->str, ";が必要です");

		tmp = calloc(1, sizeof(MemberElem));
		tmp->name = ident->str;
		tmp->name_len = ident->len;
		tmp->type = type;
		tmp->next = def->members;
		tmp->offset = 0;

		// 型のサイズを取得
		typesize = get_type_size(type);
		if (typesize == -1)
			error_at(ident->str, "型のサイズが確定していません");
		def->mem_size = max(def->mem_size, typesize);

		def->members = tmp;
	}

	debug("#  MEMSIZE = %d\n", def->mem_size);
	return (new_node(ND_UNION_DEF, NULL, NULL));
}


// TODO ブロックを抜けたらlocalsを戻す
// TODO 変数名の被りチェックは別のパスで行う
// (まで読んだところから読む
Node	*funcdef(Env *env, Type *type, Token *ident, bool is_static)
{
	Node	*node;
	Token	*arg;
	int		i;
	int		type_size;
	LVar	*lvar;

	env->locals = NULL;

	// create node
	node = new_node(ND_FUNCDEF, NULL, NULL);
	node->fname = strndup(ident->str, ident->len);
	node->flen = ident->len;
	node->ret_type = type;
	node->argdef_count = 0;
	node->is_variable_argument = false;
	node->is_static = is_static;
	env->func_now = node;

	// 返り値がMEMORYならダミーの変数を追加する
	if (is_memory_type(node->ret_type))
	{
		// RDIを保存する用	
		lvar = calloc(1, sizeof(LVar));
		lvar->name = "";
		lvar->len = 0;
		lvar->type = new_type_ptr_to(new_primitive_type(TY_VOID));
		lvar->is_arg = true;
		lvar->arg_regindex = 0;
		lvar->is_dummy = true;
		lvar->offset = 8;
		lvar->next = NULL;
		env->locals = lvar;

		// とりあえずalign(typesize,16) * 2だけ隙間を用意しておく
		// しかし使わない
		type_size = align_to(get_type_size(node->ret_type), 16);

		lvar = calloc(1, sizeof(LVar));
		lvar->name = "";
		lvar->len = 0;
		lvar->type = node->ret_type;
		lvar->is_arg = false;
		lvar->arg_regindex = -1;
		lvar->is_dummy = true;
		lvar->offset = 8 + type_size * 2;
		lvar->next = NULL;
		env->locals->next = lvar;
	}

	// args
	if (!consume(env, ")"))
	{
		for (;;)
		{
			// variable argument
			if (node->argdef_count != 0 && consume(env, "..."))
			{
				if (!consume(env, ")"))
					error_at(env->token->str, ")が必要です");
				node->is_variable_argument = true;
				break ;
			}

			// 型宣言の確認
			type = consume_type_before(env, false);
			if (type == NULL)
				error_at(env->token->str,"型宣言が必要です\n (funcdef)");

			// 仮引数名
			arg = consume_ident(env);
			if (arg == NULL)
			{
				// voidなら引数0個
				if (type->ty == TY_VOID && node->argdef_count == 0)
				{
					if (!consume(env, ")"))
						error_at(env->token->str, ")が見つかりませんでした。");
					break ;
				}
				error_at(env->token->str, "仮引数が必要です");
			}
			// LVarを作成
			create_local_var(env, arg->str, arg->len, type, true);
			// arrayを読む
			expect_type_after(env, &type);

			type = type_array_to_ptr(type); // 配列からポインタにする

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
		// ?
		node->argdef_count = -1;
	}

	debug("# END READ ARGS\n");

	// func_defsに代入
	// TODO 関数名被り
	// TODO プロトタイプ宣言後の関数定義
	if (consume(env, ";"))
	{
		node->kind = ND_PROTOTYPE;

		i = 0;
		while (env->func_protos[i])
			i += 1;
		env->func_protos[i] = node;
		node->locals = env->locals;
	}
	else
	{
		i = 0;
		while (env->func_defs[i])
			i += 1;
		env->func_defs[i] = node;
		node->locals = env->locals;
		node->lhs = stmt(env);
		node->locals = env->locals;
	}

	env->func_now = NULL;
	
	debug("# CREATED FUNC %s\n", strndup(node->fname, node->flen));

	return node;
}

Node	*read_typedef(Env *env)
{
	Type		*type;
	Token		*token;
	TypedefPair	*pair;

	// 型を読む
	type = consume_type_before(env, true);
	if (type == NULL)
		error_at(env->token->str, "型宣言が必要です\n (read_typedef)");
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

Node	*filescope(Env *env)
{
	Token	*ident;
	Type	*type;
	bool	is_static;
	bool	is_inline;
	Node	*node;

	// typedef
	if (consume_with_type(env, TK_TYPEDEF))
	{
		return (read_typedef(env));
	}

	// extern
	if (consume_with_type(env, TK_EXTERN))
	{
		type = consume_type_before(env, true);
		if (type == NULL)
			error_at(env->token->str, "型が必要です");
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です");
		return (global_var(env, type, ident, true, false));
	}

	is_static = false;
	if (consume_with_type(env, TK_STATIC))
	{
		is_static = true;
	}

	// TODO とりあえず無視
	is_inline = false;
	if (consume_with_type(env, TK_INLINE))
	{
		is_inline = true;
	}

	// structの宣言か返り値がstructか
	if (consume_with_type(env, TK_STRUCT))
	{
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です");
			
		if (consume(env, "{"))
		{
			node = read_struct_block(env, ident);
			// ;なら構造体の宣言
			// そうでないなら返り値かグローバル変数
			if (consume(env, ";"))
				return (node);
		}
		type = new_struct_type(env, ident->str, ident->len);
		consume_type_ptr(env, &type);
	}
	// enumの宣言か返り値がenumか
	else if (consume_with_type(env, TK_ENUM))
	{
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です");
			
		if (consume(env, "{"))
		{
			node = read_enum_block(env, ident);
			// ;ならenumの宣言
			// そうでないなら返り値かグローバル変数
			if (consume(env, ";"))
				return (node);
		}
		type = new_enum_type(env, ident->str, ident->len);
		consume_type_ptr(env, &type);
	}
	// unionの宣言か返り値がunionか
	else if (consume_with_type(env, TK_UNION))
	{
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "識別子が必要です");
			
		if (consume(env, "{"))
		{
			node = read_union_block(env, ident);
			// ;ならunionの宣言
			// そうでないなら返り値かグローバル変数
			if (consume(env, ";"))
				return (node);
		}
		type = new_union_type(env, ident->str, ident->len);
		consume_type_ptr(env, &type);
	}
	else
		type = consume_type_before(env, true);

	// TODO 一旦staticは無視
	// グローバル変数か関数宣言か
	if (type != NULL)
	{
		// ident
		ident = consume_ident(env);
		if (ident == NULL)
			error_at(env->token->str, "不明なトークンです");

		// function definition
		if (consume(env, "("))
			return funcdef(env, type, ident, is_static);
		else
			return global_var(env, type, ident, false, is_static);
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

Env	*parse(Token *tok)
{
	Env	*env;

	env = calloc(1, sizeof(Env));
	env->token = tok;
	env->type_alias = linked_list_new();
	program(env);
	return env;
}
