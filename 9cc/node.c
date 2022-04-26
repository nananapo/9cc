#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern Node		*code[];
extern Token	*token;
extern Node		*func_defs[];
extern Node		*func_protos[];

LVar		*locals;

LVar	*find_lvar(char *str, int len)
{
	for (LVar *var = locals; var; var = var->next)
		if (var->len == len && memcmp(str, var->name, var->len) == 0)
			return var;
	return NULL;
}

int	type_size(Type *type)
{
	if (type->ty == INT)
		return 8;
	if (type->ty == PTR)
		return 8;
	return -1;
}

int	create_local_var(char *name, int len, Type *type)
{
	// TODO check same
	LVar *lvar = calloc(1, sizeof(LVar));
	lvar->next = locals;
	lvar->name = name;
	lvar->len = len;
	lvar->type = type;
	if (locals == NULL)
		lvar->offset = type_size(type);
	else
		lvar->offset = locals->offset + type_size(type);
	locals = lvar;
	return lvar->offset;
}

int	get_locals_count()
{
	int	i = 0;
	LVar *tmp = locals;
	while (tmp)
	{
		i++;
		tmp = tmp->next;
	}
	return i;
}

Node	*get_function_by_name(char *name, int len)
{
	int i = 0;
	while (func_defs[i])
	{
		Node *tmp = func_defs[i];
		if (tmp->flen == len && strncmp(tmp->fname, name, len) == 0)
			return tmp;
		i++;
	}

	i = 0;
	while (func_protos[i])
	{
		Node *tmp = func_protos[i];
		if (tmp->flen == len && strncmp(tmp->fname, name, len) == 0)
			return tmp;
		i++;
	}
	return NULL;
}

Type	*new_primitive_type(PrimitiveType pri)
{
	Type	*type = calloc(1, sizeof(Type));
	type->ty = pri;
	type->ptr_to = NULL;
	type->next = NULL;
	return type;
}

Type	*new_type_ptr_to(Type *ptr_to)
{
	Type	*type = new_primitive_type(PTR);
	type->ptr_to = ptr_to;
	return type;
}

// 2つのTypeが一致するかどうか
int	type_equal(Type *t1, Type *t2)
{
	if (t1->ty != t2->ty)
		return 0;
	if (t1->ty == PTR)
		return type_equal(t1->ptr_to, t2->ptr_to);
	return 1;
}

Type	*consume_defident_type()
{
	// type
	if (!consume_ident_str("int"))
		return NULL;

	Type *type = new_primitive_type(INT);
	while (consume("*"))
	{
		Type *tmp = new_primitive_type(PTR);
		tmp->ptr_to = type;
		type = tmp;
	}

	return type;
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

Node *new_node_num(int val)
{
	Node *node = new_node(ND_NUM, NULL, NULL);
	node->val = val;
	node->type = new_primitive_type(INT);
	return node;
}

Node *primary()
{
	Token	*tok;

	// 括弧
	if (consume("("))
	{
		Node	*node = expr();
		expect(")");
		return node;
	}
	
	// identかcall
	tok = consume_ident();
	if (tok)
	{
		// call func
		if (consume("("))
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

		// use ident
		Node	*node = new_node(ND_LVAR, NULL, NULL);
		LVar	*lvar = find_lvar(tok->str, tok->len);

		if (lvar == NULL)
			error_at(tok->str, "%sが定義されていません", strndup(tok->str, tok->len));
		
		node->offset = lvar->offset;
		node->type = lvar->type;
		return node;
	}

	// 数字
	return new_node_num(expect_number());
}

Node *unary()
{
	Node	*node;
	if (consume("+"))
	{
		node = primary();
		if (node->type->ty == PTR)
			error_at(token->str, "ポインタ型に対してunary -を適用できません");
		return node;
	}
	else if (consume("-"))
	{
		node = new_node(ND_SUB, new_node_num(0), primary());
		if (node->rhs->type->ty == PTR)
			error_at(token->str, "ポインタ型に対してunary -を適用できません");
		node->type = node->rhs->type;
		return node;
	}
	else if (consume("*"))
	{
		node = new_node(ND_DEREF, unary(), NULL);
		if (node->lhs->type->ty != PTR)
			error_at(token->str, "ポインタではない型に対してunary *を適用できません");
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
	return primary();
}

Node *mul()
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

		if (node->lhs->type->ty != INT || node->rhs->type->ty != INT)
			error_at(token->str, "ポインタ型に* か / を適用できません");
		node->type = new_primitive_type(INT);
	}
}

Node *add()
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
		
		// TODO 片方がポインタならポインタ型にする
		if (node->lhs->type->ty != node->rhs->type->ty)
		{
			if (node->lhs->type->ty == PTR)
				node->type = node->lhs->type;
			else
				node->type = node->rhs->type;
		}
		else
		{
			if (node->lhs->type->ty == PTR)
				error_at(token->str, "ポインタ型とポインタ型に+か-を適用できません");
			// とりあえずINT
			node->type = new_primitive_type(INT);
		}
	}
}

Node *relational()
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

		if (node->lhs->type->ty != node->rhs->type->ty)
			error_at(token->str, "ポインタ型とintを比較することはできません");
	}
}

Node *equality()
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

		if (node->lhs->type->ty != node->rhs->type->ty)
			error_at(token->str, "ポインタ型とintを比較することはできません");
	}
}

Node	*assign()
{
	Node	*node = equality();
	if (consume("="))
	{
		node = new_node(ND_ASSIGN, node, assign());
		if (node->lhs->type->ty != node->rhs->type->ty)
			error_at(token->str, "左辺と右辺の型が一致しません");
		node->type = node->lhs->type;
	}
	return node;
}

Node	*expr()
{
	return assign();
}

Node	*stmt()
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
		Type	*type = consume_defident_type();
		if (type != NULL)
		{
			Token *tok = consume_ident();
			if (tok == NULL)
				error_at(token->str, "識別子が必要です");
			node = new_node(ND_DEFVAR, NULL, NULL);
			node->offset = create_local_var(tok->str, tok->len, type);
		}
		else
		{
			node = expr();
		}
	}
	if(!consume(";"))
		error_at(token->str, ";ではないトークンです");

	//TODO type

	return node;
}

Node	*filescope()
{
	Node	*node;
	Token	*tok;
	locals = NULL;

	// return type
	Type	*ret_type = consume_defident_type();
	if (ret_type == NULL)
		error_at(token->str, "型宣言が必要です");

	// function name
	tok = consume_ident();
	if (tok == NULL)
		error_at(token->str, "不明なトークンです");

	// create node
	node = new_node(ND_FUNCDEF, NULL, NULL);
	node->fname = strndup(tok->str, tok->len);
	node->flen = tok->len;
	node->ret_type = ret_type;
	node->argdef_count = 0;
	// TODO type

	// args
	if (!consume("("))
		error_at(token->str, "(ではないトークンです");
	if (!consume(")"))
	{
		for (;;)
		{
			// 型宣言の確認
			Type *type = consume_defident_type();
			if (type == NULL)
				error_at(token->str,"型宣言が必要です");

			// 型情報を保存
			type->next = node->arg_type;
			node->arg_type = type;
			node->argdef_count++;

			// 仮引数名
			Token *arg = consume_ident();
			if (arg == NULL)
				error_at(token->str, ")ではないトークンです");
			create_local_var(arg->str, arg->len, type);
			
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

void	program()
{
	int	i;

	i = 0;
	while (!at_eof())
		code[i++] = filescope();
	code[i] = NULL;
}
