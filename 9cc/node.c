#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

extern Node		*code[];
extern Token	*token;
extern char		*func_defs[];

LVar		*locals;

LVar	*find_lvar(Token *tok)
{
	for (LVar *var = locals; var; var = var->next)
		if (var->len == tok->len && memcmp(tok->str, var->name, var->len) == 0)
			return var;
	return NULL;
}

int	create_local_var(char *name, int len)
{
	LVar *lvar = calloc(1, sizeof(LVar));
	lvar->next = locals;
	lvar->name = name;
	lvar->len = len;
	if (locals == NULL)
		lvar->offset = 8;
	else
		lvar->offset = locals->offset + 8;
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

int	is_block_node(Node *node)
{
	switch (node->kind)
	{
		case ND_BLOCK:
		case ND_IF:
		case ND_WHILE:
		case ND_FOR:
		case ND_RETURN:
			return true;
		default:
			return false;
	}
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
	return node;
}

Node *new_node_num(int val)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *primary()
{
	Token	*tok;

	if (consume("("))
	{
		Node	*node = expr();
		expect(")");
		return node;
	}
	tok = consume_ident();
	if (tok)
	{
		if (consume("("))
		{
			Node	*node = new_node(ND_CALL, NULL, NULL);
			node->fname = tok->str;
			node->flen = tok->len;
			node->args = NULL;
			if (!consume(")"))
			{
				Node *arg = new_node(ND_DUMMY, expr(), NULL);
				node->args = arg;
				for (;;)
				{	
					if (consume(")"))
						break;
					if (!consume(","))
						error_at(token->str, "トークンが,ではありません");
					arg->rhs = new_node(ND_DUMMY, expr(), NULL);
					arg = arg->rhs;
				}
			}
			return node;
		}
		Node	*node = new_node(ND_LVAR, NULL, NULL);
		LVar	*lvar = find_lvar(tok);
		if (lvar)
			node->offset = lvar->offset;
		else
			node->offset = create_local_var(tok->str, tok->len);
		return node;
	}
	return new_node_num(expect_number());
}

Node *unary()
{
	if (consume("+"))
		return primary();
	if (consume("-"))
		return  new_node(ND_SUB, new_node_num(0), primary());
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
	}
}

Node	*assign()
{
	Node	*node = equality();
	if (consume("="))
		node = new_node(ND_ASSIGN, node, assign());
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
		node = new_node(ND_RETURN, expr(), NULL);
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
		node = expr();
	}
	if(!consume(";"))
		error_at(token->str, ";ではないトークンです");
	return node;
}

Node	*filescope()
{
	Node	*node;
	Token	*tok = consume_ident();
	locals = NULL;
	
	if (tok != NULL)
	{
		node = new_node(ND_FUNCDEF, NULL, NULL);
		node->fname = tok->str;
		node->flen = tok->len;
		node->argdef_count = 0;

		if (!consume("("))
			error_at(token->str, "(ではないトークンです");

		// TODO same name args check
		for (;;)
		{
			if (consume(")"))
				break;
			Token *arg = consume_ident();
			if (arg == NULL)
				error_at(token->str, ")ではないトークンです");
			create_local_var(arg->str, arg->len);
			node->argdef_count++;
			if (consume(")"))
				break;
			if (consume(","))
				error_at(token->str, ",が必要です");
		}
		node->lhs = stmt();
		node->locals_len = get_locals_count();
		
		int i = 0;
		while (func_defs[i])
			i += 1;
		func_defs[i] = strndup(node->fname, node->flen);
		
		return node;
	}
	error_at(token->str, "不明なトークンです");
	return NULL;
}

void	program()
{
	int	i;

	i = 0;
	while (!at_eof())
		code[i++] = filescope();
	code[i] = NULL;
}
