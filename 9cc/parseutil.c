#include "9cc.h"
#include "string.h"
#include "stdlib.h"

extern Token *token;

bool	consume(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len) != 0)
		return false;
	token = token->next;
	return true;
}

bool consume_number(int *result)
{
	if (token->kind != TK_NUM)
		return false;
	*result = token->val;
	token = token->next;
	return true;	
}

bool	consume_with_type(TokenKind kind)
{
	if (token->kind != kind)
		return false;
	token = token->next;
	return true;
}

Token	*consume_ident()
{
	Token	*ret;
	if (token->kind != TK_IDENT)
		return NULL;
	ret = token;
	token = token->next;
	return ret;
}

Token	*consume_ident_str(char *p)
{
	Token	*ret;
	if (token->kind != TK_IDENT)
		return NULL;
	if (strncmp(p, token->str, strlen(p)) != 0)
		return NULL;
	ret = token;
	token = token->next;
	return ret;
}

Token	*consume_str_literal()
{
	Token	*ret;
	if (token->kind != TK_STR_LITERAL)
		return NULL;
	ret = token;
	token = token->next;
	return ret;
}

Token	*consume_char_literal()
{
	Token	*ret;
	if (token->kind != TK_CHAR_LITERAL)
		return NULL;
	ret = token;
	token = token->next;
	return ret;
}

void	consume_type_ptr(Type **type)
{
	Type	*tmp;

	while (consume("*"))
	{
		tmp = new_primitive_type(PTR);
		tmp->ptr_to = *type;
		*type = tmp;
	}
}

// 型宣言の前部分 (type ident arrayのtype部分)を読む
Type	*consume_type_before()
{
	Type	*type;
	Token	*ident;

	// type name
	if (consume_ident_str("int"))
		type = new_primitive_type(INT);
	else if (consume_ident_str("char"))
		type = new_primitive_type(CHAR);
	else if (consume_ident_str("void"))
		type = new_primitive_type(VOID);
	else if (consume_with_type(TK_STRUCT))
	{
		ident = consume_ident();
		if (ident == NULL)
			return (NULL);
		type = new_struct_type(ident->str, ident->len);
		if (type == NULL)
			return (NULL);
	}
	else
		return (NULL);

	consume_type_ptr(&type);
	return type;
}

// 型宣言のarray部分を読む
void	expect_type_after(Type **type)// expect size
{
	int		size;

	while (consume("["))
	{
		*type = new_type_array(*type);
		if (!consume_number(&size))
			error_at(token->str, "配列のサイズが定義されていません");
		(*type)->array_size = size;
		if (!consume("]"))
			error_at(token->str, "]がありません");
	}
	return;
}

void	expect(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len) != 0)
		error_at(token->str, "'%s'ではありません", op);
	token = token->next;
}

int expect_number()
{
	int val;

	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
	val = token->val;
	token = token->next;
	return val;	
}

bool	at_eof()
{
	return token->kind == TK_EOF;
}

