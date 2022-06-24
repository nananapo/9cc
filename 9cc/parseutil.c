#include "9cc.h"
#include <string.h>
#include <stdbool.h>

#define Env ParseResult

bool	consume(Env *env, char *op)
{
	if (env->token->kind != TK_RESERVED ||
		strlen(op) != env->token->len ||
		memcmp(env->token->str, op, env->token->len) != 0)
		return false;
	env->token = env->token->next;
	return true;
}

bool consume_number(Env *env, int *result)
{
	if (env->token->kind != TK_NUM)
		return false;
	*result = env->token->val;
	env->token = env->token->next;
	return true;	
}

bool	consume_with_type(Env *env, TokenKind kind)
{
	if (env->token->kind != kind)
		return false;
	env->token = env->token->next;
	return true;
}

Token	*consume_ident(Env *env)
{
	Token	*ret;
	if (env->token->kind != TK_IDENT)
		return NULL;
	ret = env->token;
	env->token = env->token->next;
	return ret;
}

Token	*consume_ident_str(Env *env, char *p)
{
	Token	*ret;
	if (env->token->kind != TK_IDENT)
		return NULL;
	if (strncmp(p, env->token->str, strlen(p)) != 0)
		return NULL;
	ret = env->token;
	env->token = env->token->next;
	return ret;
}

Token	*consume_str_literal(Env *env)
{
	Token	*ret;
	if (env->token->kind != TK_STR_LITERAL)
		return NULL;
	ret = env->token;
	env->token = env->token->next;
	return ret;
}

Token	*consume_char_literal(Env *env)
{
	Token	*ret;
	if (env->token->kind != TK_CHAR_LITERAL)
		return NULL;
	ret = env->token;
	env->token = env->token->next;
	return ret;
}

void	consume_type_ptr(Env *env, Type **type)
{
	Type	*tmp;

	while (consume(env, "*"))
	{
		tmp = new_primitive_type(PTR);
		tmp->ptr_to = *type;
		*type = tmp;
	}
}

// 型宣言の前部分 (type ident arrayのtype部分)を読む
Type	*consume_type_before(Env *env)
{
	Type	*type;
	Token	*ident;

	// type name
	if (consume_ident_str(env, "int"))
		type = new_primitive_type(INT);
	else if (consume_ident_str(env, "char"))
		type = new_primitive_type(CHAR);
	else if (consume_ident_str(env, "void"))
		type = new_primitive_type(VOID);
	else if (consume_with_type(env, TK_STRUCT))
	{
		ident = consume_ident(env);
		if (ident == NULL)
			return (NULL);
		type = new_struct_type(env, ident->str, ident->len);
		if (type == NULL)
			return (NULL);
	}
	else
		return (NULL);

	consume_type_ptr(env, &type);
	return type;
}

// 型宣言のarray部分を読む
void	expect_type_after(Env *env, Type **type)// expect size
{
	int		size;

	while (consume(env, "["))
	{
		*type = new_type_array(*type);
		if (!consume_number(env, &size))
			error_at(env->token->str, "配列のサイズが定義されていません");
		(*type)->array_size = size;
		if (!consume(env, "]"))
			error_at(env->token->str, "]がありません");
	}
	return;
}

void	expect(Env *env, char *op)
{
	if (env->token->kind != TK_RESERVED ||
		strlen(op) != env->token->len ||
		memcmp(env->token->str, op, env->token->len) != 0)
		error_at(env->token->str, "'%s'ではありません", op);
	env->token = env->token->next;
}

int expect_number(Env *env)
{
	int val;

	if (env->token->kind != TK_NUM)
		error_at(env->token->str, "数ではありません");
	val = env->token->val;
	env->token = env->token->next;
	return val;	
}

bool	at_eof(Env *env)
{
	return env->token->kind == TK_EOF;
}
