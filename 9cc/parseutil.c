#include "9cc.h"
#include <string.h>
#include <stdbool.h>

#define Env ParseResult

Node	*read_struct_block(Env *env, Token *ident);
Node	*read_enum_block(Env *env, Token *ident);
Node	*read_union_block(Env *env, Token *ident);

bool	consume(Env *env, char *op)
{
	if (env->token->kind != TK_RESERVED ||
		(int)strlen(op) != env->token->len ||
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
		tmp = new_primitive_type(TY_PTR);
		tmp->ptr_to = *type;
		*type = tmp;
	}
}

static bool	consume_type_alias(Env *env, Type **type)
{
	TypedefPair	*pair;

	if (env->token->kind != TK_IDENT)
		return (false);
	pair = linked_list_search(env->type_alias, strndup(env->token->str, env->token->len));
	if (pair == NULL)
		return (false);
	consume_ident(env);
	*type = pair->type;
	return (true);
}

// 型宣言の前部分 (type ident arrayのtype部分)を読む
// read_def	: structの宣言を読むかどうか
Type	*consume_type_before(Env *env, bool read_def)
{
	Type	*type;
	Token	*ident;

	// type name
	if (consume_ident_str(env, "int"))
		type = new_primitive_type(TY_INT);
	else if (consume_ident_str(env, "char"))
		type = new_primitive_type(TY_CHAR);
	else if (consume_ident_str(env, "_Bool"))
		type = new_primitive_type(TY_BOOL);
	else if (consume_ident_str(env, "void"))
		type = new_primitive_type(TY_VOID);
	else if (consume_with_type(env, TK_STRUCT))
	{
		ident = consume_ident(env);
		if (ident == NULL)
			return (NULL);
		if (read_def && consume(env, "{"))
			read_struct_block(env, ident);

		type = new_struct_type(env, ident->str, ident->len);
		if (type == NULL)
			return (NULL);
	}
	else if (consume_with_type(env, TK_ENUM))
	{
		ident = consume_ident(env);
		if (ident == NULL)
			return (NULL);
		if (read_def && consume(env, "{"))
			read_enum_block(env, ident);

		type = new_enum_type(env, ident->str, ident->len);
		if (type == NULL)
			return (NULL);
	}
	else if (consume_with_type(env, TK_UNION))
	{
		ident = consume_ident(env);
		if (ident == NULL)
			return (NULL);
		if (read_def && consume(env, "{"))
			read_union_block(env, ident);

		type = new_union_type(env, ident->str, ident->len);
		if (type == NULL)
			return (NULL);
	}
	else if (consume_type_alias(env, &type))
	{
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
		(int)strlen(op) != env->token->len ||
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
