#include "9cc.h"
#include <string.h>
#include <stdbool.h>

Node	*read_struct_block(Token *ident);
Node	*read_enum_block(Token *ident);
Node	*read_union_block(Token *ident);

// main
extern Token			*g_token;
extern t_deffunc		*g_func_defs[1000];
extern t_deffunc		*g_func_protos[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals;
extern StructDef		*g_struct_defs[1000];
extern EnumDef			*g_enum_defs[1000];
extern UnionDef			*g_union_defs[1000];
extern LVar				*g_locals;
extern t_deffunc		*g_func_now;
extern t_linked_list	*g_type_alias;

bool	consume(char *op)
{
	if (g_token->kind != TK_RESERVED ||
		(int)strlen(op) != g_token->len ||
		memcmp(g_token->str, op, g_token->len) != 0)
		return false;
	g_token = g_token->next;
	return true;
}

bool consume_number(int *result)
{
	if (g_token->kind != TK_NUM)
		return false;
	*result = g_token->val;
	g_token = g_token->next;
	return true;	
}

bool	consume_with_type(TokenKind kind)
{
	if (g_token->kind != kind)
		return false;
	g_token = g_token->next;
	return true;
}

Token	*consume_ident(void)
{
	Token	*ret;
	if (g_token->kind != TK_IDENT)
		return NULL;
	ret = g_token;
	g_token = g_token->next;
	return ret;
}

Token	*consume_ident_str(char *p)
{
	Token	*ret;
	if (g_token->kind != TK_IDENT)
		return NULL;
	if (strncmp(p, g_token->str, strlen(p)) != 0)
		return NULL;
	ret = g_token;
	g_token = g_token->next;
	return ret;
}

Token	*consume_str_literal(void)
{
	Token	*ret;
	if (g_token->kind != TK_STR_LITERAL)
		return NULL;
	ret = g_token;
	g_token = g_token->next;
	return ret;
}

Token	*consume_char_literal(void)
{
	Token	*ret;
	if (g_token->kind != TK_CHAR_LITERAL)
		return NULL;
	ret = g_token;
	g_token = g_token->next;
	return ret;
}

void	consume_type_ptr(Type **type)
{
	Type	*tmp;

	while (consume("*"))
	{
		tmp = new_primitive_type(TY_PTR);
		tmp->ptr_to = *type;
		*type = tmp;
	}
}

static bool	consume_type_alias(Type **type)
{
	TypedefPair	*pair;

	if (g_token->kind != TK_IDENT)
		return (false);
	pair = linked_list_search(g_type_alias, strndup(g_token->str, g_token->len));
	if (pair == NULL)
		return (false);
	consume_ident();
	*type = pair->type;
	return (true);
}

// 型宣言の前部分 (type ident arrayのtype部分)を読む
// read_def	: structの宣言を読むかどうか
Type	*consume_type_before(bool read_def)
{
	Type	*type;
	Token	*ident;

	// type name
	if (consume_ident_str("int"))
		type = new_primitive_type(TY_INT);
	else if (consume_ident_str("char"))
		type = new_primitive_type(TY_CHAR);
	else if (consume_ident_str("_Bool"))
		type = new_primitive_type(TY_BOOL);
	else if (consume_ident_str("void"))
		type = new_primitive_type(TY_VOID);
	else if (consume_with_type(TK_STRUCT))
	{
		ident = consume_ident();
		if (ident == NULL)
			return (NULL);
		if (read_def && consume("{"))
			read_struct_block(ident);

		type = new_struct_type(ident->str, ident->len);
		if (type == NULL)
			return (NULL);
	}
	else if (consume_with_type(TK_ENUM))
	{
		ident = consume_ident();
		if (ident == NULL)
			return (NULL);
		if (read_def && consume("{"))
			read_enum_block(ident);

		type = new_enum_type(ident->str, ident->len);
		if (type == NULL)
			return (NULL);
	}
	else if (consume_with_type(TK_UNION))
	{
		ident = consume_ident();
		if (ident == NULL)
			return (NULL);
		if (read_def && consume("{"))
			read_union_block(ident);

		type = new_union_type(ident->str, ident->len);
		if (type == NULL)
			return (NULL);
	}
	else if (consume_type_alias(&type))
	{
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
			error_at(g_token->str, "配列のサイズが定義されていません");
		(*type)->array_size = size;
		if (!consume("]"))
			error_at(g_token->str, "]がありません");
	}
	return;
}

bool	consume_enum_key(Type **type, int *value)
{
	Token	*tok;
	EnumDef	*res_def;

	if (g_token->kind != TK_IDENT)
		return (false);
	tok = g_token;
	if (find_enum(tok->str, tok->len, &res_def, value))
	{
		consume_ident();
		if (type != NULL)
			*type = new_enum_type(res_def->name, res_def->name_len);
		return (true);
	}
	return (false);
}

bool consume_charlit(int *number)
{
	if (g_token->kind != TK_CHAR_LITERAL)
		return (false);

	*number = get_char_to_int(g_token->str, g_token->strlen_actual);
	if (*number == -1)
		error_at(g_token->str, "不明なエスケープシーケンスです");

	g_token = g_token->next; // 進める
	return (true);
}

void	expect_semicolon(void)
{
	if (consume(";"))
		return ;
	error_at(g_token->str, "; expected.");
}

void	expect(char *op)
{
	if (g_token->kind != TK_RESERVED ||
		(int)strlen(op) != g_token->len ||
		memcmp(g_token->str, op, g_token->len) != 0)
		error_at(g_token->str, "'%s'ではありません", op);
	g_token = g_token->next;
}

int expect_number(void)
{
	int val;

	if (g_token->kind != TK_NUM)
		error_at(g_token->str, "数ではありません");
	val = g_token->val;
	g_token = g_token->next;
	return val;	
}

bool	at_eof(void)
{
	return g_token->kind == TK_EOF;
}
