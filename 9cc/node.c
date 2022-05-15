#include "9cc.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

extern Node			*code[];
extern Token		*token;
extern Node			*func_defs[];
extern Node			*func_protos[];
extern Node			*global_vars[];
extern t_str_elem	*str_literals;
extern StructDef	*struct_defs[];

LVar				*locals;

// Node
static Node	*expr();

Node		*get_function_by_name(char *name, int len);
Node		*new_node_num(int val);

// LVar
LVar		*find_lvar(char *str, int len);
int			create_local_var(char *name, int len, Type *type, bool is_arg);
int			get_locals_count();

// リテラルを探す
static int	get_str_literal_index(char *str, int len)
{
	t_str_elem	*tmp;

	tmp = str_literals;
	while (tmp != NULL)
	{
		if (len == tmp->len && strncmp(tmp->str, str, len))
			return (tmp->index);
		tmp = tmp->next;
	}
	tmp = malloc(sizeof(t_str_elem));
	tmp->str = str;
	tmp->len = len;
	if (str_literals == NULL)
		tmp->index = 0;
	else
		tmp->index = str_literals->index + 1;
	tmp->next = str_literals;
	str_literals = tmp;
	return (tmp->index);
}

static Node *call(Token *tok)
{
	Node	*node = new_node(ND_CALL, NULL, NULL);

	node->fname = tok->str;
	node->flen = tok->len;
	node->args = NULL;
	node->argdef_count = 0;

	if (!consume(")"))
	{
		Node *arg = expr();
		node->args = arg;
		node->argdef_count = 1;

		for (;;)
		{	
			if (consume(")"))
				break;
			if (!consume(","))
				error_at(token->str, "トークンが,ではありません");
			
			arg = expr();
			arg->next = node->args;
			node->args = arg;
			node->argdef_count += 1;
		}
	}
	
	// TODO これは違うパスでいいかも
	Node *refunc = get_function_by_name(node->fname, node->flen);

	// 関数定義が見つからない場合
	if (refunc == NULL)
		error_at(token->str, "warning : 関数%sがみつかりません\n", strndup(node->fname, node->flen));
	else
	{
		// 引数の数を確認
		if (node->argdef_count != refunc->argdef_count)
			error_at(token->str, "関数%sの引数の数が一致しません", strndup(node->fname, node->flen));

		Type *def = refunc->arg_type;
		Node *use = node->args;
		while (def != NULL)
		{
			if (!type_equal(def, use->type))
				error_at(token->str, "関数%sの引数の型が一致しません", strndup(node->fname, node->flen));
			def = def->next;
			use = use->next;
		}

		// 型を返り値の型に設定
		node->type = refunc->ret_type;
	}
	return node;
}

// 添字によるDEREF
static Node	*read_deref_index(Node *node)
{
	while (consume("["))
	{
		Node	*add = new_node(ND_ADD, node, expr());

		// 左にポインタを寄せる
		if (is_pointer_type(add->rhs->type))
		{
			Node	*tmp;
			tmp = add->lhs;
			add->lhs = add->rhs;
			add->rhs = tmp;
		}

		if (!is_pointer_type(add->lhs->type))
			error_at(token->str, "ポインタ型ではありません");
		if (!is_integer_type(add->rhs->type))
			error_at(token->str, "添字の型が整数ではありません");
		add->type = add->lhs->type;

		node = new_node(ND_DEREF, add, NULL);
		node->type = node->lhs->type->ptr_to;

		if (!consume("]"))
			error_at(token->str, "%s");
	}
	return (node);
}

static Node *primary()
{
	Token	*tok;
	Node	*node;

	// 括弧
	if (consume("("))
	{
		node = expr();
		expect(")");
		return node;
	}
	
	// identかcall
	tok = consume_ident();
	if (tok)
	{
		// call func
		if (consume("("))
			node = call(tok);
		// use ident
		else
		{
			LVar	*lvar = find_lvar(tok->str, tok->len);
			if (lvar != NULL)
			{
				node = new_node(ND_LVAR, NULL, NULL);
				node->offset = lvar->offset;
				node->type = lvar->type;
			}
			else
			{
				Node *glovar = find_global(tok->str, tok->len);
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
		return read_deref_index(node);
	}

	// string
	tok = consume_str_literal();
	if (tok)
	{
		node = new_node(ND_STR_LITERAL, NULL, NULL);
		node->str_index = get_str_literal_index(tok->str, tok->len);
		node->type = new_type_ptr_to(new_primitive_type(CHAR));
		return read_deref_index(node);
	}

	// 数
	int number;
	if (!consume_number(&number))
		error_at(token->str, "数字が必要です");
	node = new_node_num(number);

	return read_deref_index(node);
}

static Node *unary()
{
	Node	*node;
	if (consume("+"))
	{
		node = primary();
		if (is_pointer_type(node->type))
			error_at(token->str, "ポインタ型に対してunary -を適用できません");
		return node;
	}
	else if (consume("-"))
	{
		node = new_node(ND_SUB, new_node_num(0), primary());
		if (is_pointer_type(node->rhs->type))
			error_at(token->str, "ポインタ型に対してunary -を適用できません");
		node->type = node->rhs->type;
		return node;
	}
	else if (consume("*"))
	{
		node = new_node(ND_DEREF, unary(), NULL);
		if (!is_pointer_type(node->lhs->type))
			error_at(token->str, "ポインタではない型(%d)に対してunary *を適用できません", node->lhs->type->ty);
		node->type = node->lhs->type->ptr_to;	
		return node;
	}
	else if (consume("&"))
	{
		node = new_node(ND_ADDR, unary(), NULL);
		// TODO 左辺値かどうかのチェック
		node->type = new_type_ptr_to(node->lhs->type);
		return node;
	}
	else if (consume_with_type(TK_SIZEOF))
	{
		node = unary();
		node = new_node_num(type_size(node->type));
		return node;
	}
	return primary();
}

static Node *mul()
{
	Node *node = unary();
	for (;;)
	{
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return node;

		if (!is_integer_type(node->lhs->type)
		|| !is_integer_type(node->rhs->type))
			error_at(token->str, "ポインタ型に* か / を適用できません");
		node->type = new_primitive_type(INT);
	}
}

static Node *add()
{
	Node *node = mul();
	for (;;)
	{
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return node;

		Type	*l;
		Type	*r;

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

		// 両方がポインタならエラー
		if (is_pointer_type(l)
		&& is_pointer_type(r))
			error_at(token->str, "ポインタ型とポインタ型に+か-を適用できません");

		// ポインタと整数の演算
		if (is_pointer_type(l)
		&& is_integer_type(r))
		{
			node->type = l;
			continue ;
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
			continue ;
		}

		error_at(token->str, "演算子 +, - が%dと%dの間に定義されていません",
				node->lhs->type->ty, node->rhs->type->ty);
	}
}

static Node *relational()
{
	Node *node = add();
	for (;;)
	{
		if (consume("<"))
			node = new_node(ND_LESS, node, add());
		else if (consume("<="))
			node = new_node(ND_LESSEQ, node, add());
		else if (consume(">"))
			node = new_node(ND_LESS, add(), node);
		else if (consume(">="))
			node = new_node(ND_LESSEQ, add(), node);
		else
			return node;

		node->type = new_primitive_type(INT);

		if (!can_compared(node->lhs->type, node->rhs->type))
			error_at(token->str,
					"%dと%dを比較することはできません",
					node->lhs->type->ty, node->rhs->type->ty);
	}
}

static Node *equality()
{
	Node *node = relational();
	for (;;)
	{
		if (consume("=="))
			node = new_node(ND_EQUAL, node, relational());
		else if (consume("!="))
			node = new_node(ND_NEQUAL, node, relational());
		else
			return node;

		node->type = new_primitive_type(INT);

		if (!can_compared(node->lhs->type, node->rhs->type))
			error_at(token->str,
					"%dと%dを比較することはできません",
					node->lhs->type->ty, node->rhs->type->ty);
	}
}

static Node	*assign()
{
	Node	*node = equality();
	if (consume("="))
	{
		node = new_node(ND_ASSIGN, node, assign());
		if (!can_assign(node->lhs->type, node->rhs->type))
			error_at(token->str, "左辺(%d)と右辺(%d)の型が一致しません",
					node->lhs->type->ty, node->rhs->type->ty);
		node->type = node->lhs->type;
	}
	return node;
}

static Node	*expr()
{
	return assign();
}

static Node	*stmt()
{
	Node	*node;

	if (consume_with_type(TK_RETURN))
	{
		// TODO 型チェック
		node = new_node(ND_RETURN, expr(), NULL);
	}
	else if (consume_with_type(TK_IF))
	{
		if (!consume("("))
			error_at(token->str, "(ではないトークンです");
		node = new_node(ND_IF, expr(), NULL);
		if (!consume(")"))
			error_at(token->str, ")ではないトークンです");
		node->rhs = stmt();
		if (consume_with_type(TK_ELSE))
			node->els = stmt();
		return node;
	}
	else if(consume_with_type(TK_WHILE))
	{
		if (!consume("("))
			error_at(token->str, "(ではないトークンです");
		node = new_node(ND_WHILE, expr(), NULL);
		if (!consume(")"))
			error_at(token->str, ")ではないトークンです");
		node->rhs = stmt();
		return node;
	}
	else if(consume_with_type(TK_FOR))
	{
		if (!consume("("))
			error_at(token->str, ")ではないトークンです");
		node = new_node(ND_FOR, NULL, NULL);
		// for init
		if (!consume(";"))
		{
			node->for_init = expr();
			if (!consume(";"))
				error_at(token->str, ";が必要です");
		}
		// for if
		if (!consume(";"))
		{
			node->for_if = expr();
			if (!consume(";"))
				error_at(token->str, ";が必要です");
		}
		// for next
		if (!consume(")"))
		{
			node->for_next = expr();
			if(!consume(")"))
				error_at(token->str, ")ではないトークンです");
		}
		// stmt
		node->lhs = stmt();
		return node;
	}
	else if(consume("{"))
	{
		node = new_node(ND_BLOCK, NULL, NULL);
		Node *start = node;
		while (!consume("}"))
		{
			node->lhs = stmt();
			node->rhs = new_node(ND_BLOCK, NULL, NULL);
			node = node->rhs;
		}
		return start;
	}
	else
	{
		Type	*type = consume_type_before();
		if (type != NULL)
		{
			Token *tok = consume_ident();

			if (tok == NULL)
				error_at(token->str, "識別子が必要です");

			expect_type_after(&type);

			node = new_node(ND_DEFVAR, NULL, NULL);
			node->offset = create_local_var(tok->str, tok->len, type, false);
		}
		else
		{
			node = expr();
		}
	}
	if(!consume(";"))
		error_at(token->str, ";ではないトークン(%d)です", token->kind);

	return node;
}

static Node	*global_var(Type *type, Token *ident)
{
	int		i;
	Node	*node;

	// 後ろの型を読む
	expect_type_after(&type);

	node = new_node(ND_GLOBAL, NULL, NULL);
	node->type = type;
	node->var_name = strndup(ident->str, ident->len);
	node->var_name_len = ident->len;

	// 代入文
	/*
	if (consume("="))
	{
		// TODO
	}
	*/

	if (!consume(";"))
		error_at(token->str, ";が必要です。");

	i = -1;
	while (global_vars[++i])
		continue;
	global_vars[i] = node;

	return node;
}

static Node	*struct_block(Token *ident)
{
	Type				*type;
	StructDef			*def;
	StructMemberElem	*tmp;
	int					i;
	int					typesize;

/*
	ident = consume_ident();
	if (ident == NULL)
		error_at(token->str, "識別子が必要です");

	if (!consume("{"))
		error_at(token->str, "{が必要です");
*/

	for (i = 0; struct_defs[i]; i++)
		continue ;
	def = calloc(1, sizeof(StructDef));
	def->name = ident->str;
	def->name_len = ident->len;
	def->mem_size = -1;
	struct_defs[i] = def;

	while (1)
	{
		if (consume("}"))
			break;
		type = consume_type_before();
		if (type == NULL)
			error_at(token->str, "型宣言が必要です");
		ident = consume_ident();
		if (ident == NULL)
			error_at(token->str, "識別子が必要です");
		expect_type_after(&type);
		if (!consume(";"))
			error_at(token->str, ";が必要です");

		tmp = calloc(1, sizeof(StructMemberElem));
		tmp->name = ident->str;
		tmp->name_len = ident->len;
		tmp->next = def->members;

		// 型のサイズを取得
		typesize = type_size(type);
		if (typesize == -1)
			error_at(ident->str, "型のサイズが確定していません");

		// offsetを設定
		if (def->members ==  NULL)
			tmp->offset = typesize;
		else
		{
			i = def->members->offset;
			if (typesize < 4)
			{
				if (i % 4 + typesize > 4)
					tmp->offset = ((i + 4) / 4 * 4) + typesize;
				else
					tmp->offset = i + typesize;
			}
			else if (typesize == 4)
				tmp->offset = ((i + 3) / 4) * 4 + typesize;
			else
				tmp->offset = ((i + 7) / 8) * 8 + typesize;
		}
		// printf("# OFFSET = %d\n", tmp->offset);
		def->members = tmp;
	}

	// サイズを決定
	if (def->members == NULL)
		def->mem_size = 0;
	else
		def->mem_size = align_to(def->members->offset, 4);

	// printf("# MEMSIZE = %d\n", def->mem_size);

	if (!consume(";"))
		error_at(token->str, ";が必要です");

	return (new_node(ND_STRUCT_DEF, NULL, NULL));
}

// TODO ブロック抜けたらlocalsを戻す

// (まで読んだところから読む
static Node	*funcdef(Type *ret_type, Token *ident)
{
	Node	*node;

	locals = NULL;

	// create node
	node = new_node(ND_FUNCDEF, NULL, NULL);
	node->fname = strndup(ident->str, ident->len);
	node->flen = ident->len;
	node->ret_type = ret_type;
	node->argdef_count = 0;

	// args
	if (!consume(")"))
	{
		for (;;)
		{
			// 型宣言の確認
			Type *type = consume_type_before();
			if (type == NULL)
				error_at(token->str,"型宣言が必要です");

			// 仮引数名
			Token *arg = consume_ident();
			if (arg == NULL)
				error_at(token->str, ")ではないトークンです");
			create_local_var(arg->str, arg->len, type, true);

			// arrayかどうかを確かめる
			expect_type_after(&type);

			// 型情報を保存
			type->next = node->arg_type;
			node->arg_type = type;
			node->argdef_count++;
			
			// )か,
			if (consume(")"))
				break;
			if (!consume(","))
				error_at(token->str, ",が必要です");
		}
	}

	// func_defsに代入
	// TODO 関数名被り
	if (consume(";"))
	{
		node->kind = ND_PROTOTYPE;

		int i = 0;
		while (func_protos[i])
			i += 1;
		func_protos[i] = node;
	}
	else
	{
		
		int i = 0;
		while (func_defs[i])
			i += 1;
		func_defs[i] = node;
		node->lhs = stmt();
		node->locals = locals;
		node->locals_len = get_locals_count();
	}
	
	return node;
}

static Node	*filescope()
{
	Token	*ident;
	Type	*ret_type;

	// structの宣言か返り値がstructか
	if (consume_with_type(TK_STRUCT))
	{
		ident = consume_ident();
		if (ident == NULL)
			error_at(token->str, "構造体の識別子が必要です");
			
		if (consume("{"))
			return struct_block(ident);

		ret_type = new_struct_type(ident->str, ident->len);
		consume_type_ptr(&ret_type);
	}
	else
		ret_type = consume_type_before();

	// グローバル変数か関数宣言か
	if (ret_type != NULL)
	{
		// ident
		ident = consume_ident();
		if (ident == NULL)
			error_at(token->str, "不明なトークンです");

		// function definition
		if (consume("("))
			return funcdef(ret_type, ident);
		else
			return global_var(ret_type, ident);
	}

	error_at(token->str, "パースに失敗しました[filescope]");
	return (NULL);
}

void	program()
{
	int	i;

	i = 0;
	while (!at_eof())
		code[i++] = filescope();
	code[i] = NULL;
}
