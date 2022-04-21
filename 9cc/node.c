#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

extern Node		*code[];
extern Token	*token;

extern LVar		*locals;

LVar	*find_lvar(Token *tok)
{
	for (LVar *var = locals; var; var = var->next)
		if (var->len == tok->len && memcmp(tok->str, var->name, var->len) == 0)
			return var;
	return NULL;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
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
		Node	*node = new_node(ND_LVAR, NULL, NULL);
		LVar	*lvar = find_lvar(tok);
		if (lvar)
		{
			node->offset = lvar->offset;
		}
		else
		{
			lvar = calloc(1, sizeof(LVar));
			lvar->next = locals;
			lvar->name = tok->str;
			lvar->len = tok->len;
			if (locals == NULL)
				lvar->offset = 0;
			else
				lvar->offset = locals->offset + 8;
			node->offset = lvar->offset;
			locals = lvar;
		}
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
		Node *rhs = stmt();
		if (consume_with_type(TK_ELSE))
		{
			rhs = new_node(ND_ELSE, rhs, stmt());
		}
		node->rhs = rhs;
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
	else
	{
		node = expr();
	}
	if(!consume(";"))
		error_at(token->str, ";ではないトークンです");
	return node;
}

void	program()
{
	int	i;

	i = 0;
	while (!at_eof())
		code[i++] = stmt();
	code[i] = NULL;
}
