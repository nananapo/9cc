#include "9cc.h"
#include <string.h>
#include <stdbool.h>

t_node	*read_struct_block(t_token *ident);
t_node	*read_enum_block(t_token *ident);
t_node	*read_union_block(t_token *ident);

// main
extern t_token			*g_token;
extern t_deffunc		*g_func_defs[1000];
extern t_deffunc		*g_func_protos[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals;
extern t_defstruct		*g_struct_defs[1000];
extern t_defenum		*g_enum_defs[1000];
extern t_defunion		*g_union_defs[1000];
extern t_lvar			*g_locals;
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

bool	consume_kind(t_tokenkind kind)
{
	if (g_token->kind != kind)
		return false;
	g_token = g_token->next;
	return true;
}

t_token	*consume_ident(void)
{
	t_token	*ret;
	if (g_token->kind != TK_IDENT)
		return NULL;
	ret = g_token;
	g_token = g_token->next;
	return ret;
}

t_token	*consume_ident_str(char *p)
{
	t_token	*ret;
	if (g_token->kind != TK_IDENT)
		return NULL;
	if (strncmp(p, g_token->str, strlen(p)) != 0)
		return NULL;
	ret = g_token;
	g_token = g_token->next;
	return ret;
}

t_token	*consume_str_literal(void)
{
	t_token	*ret;
	if (g_token->kind != TK_STR_LITERAL)
		return NULL;
	ret = g_token;
	g_token = g_token->next;
	return ret;
}

t_token	*consume_char_literal(void)
{
	t_token	*ret;
	if (g_token->kind != TK_CHAR_LITERAL)
		return NULL;
	ret = g_token;
	g_token = g_token->next;
	return ret;
}

void	consume_type_ptr(t_type **type)
{
	t_type	*tmp;

	while (consume("*"))
	{
		tmp = new_primitive_type(TY_PTR);
		tmp->ptr_to = *type;
		*type = tmp;
	}
}

static bool	consume_type_alias(t_type **type)
{
	t_typedefpair	*pair;

	if (g_token->kind != TK_IDENT)
		return (false);
	pair = linked_list_search(g_type_alias, my_strndup(g_token->str, g_token->len));
	if (pair == NULL)
		return (false);
	consume_ident();
	*type = pair->type;
	return (true);
}

t_token	*consume_any(void)
{
	t_token	*ret;
	ret = g_token;
	g_token = g_token->next;
	return ret;
}

t_type	*consume_type_specifier(void)
{
	t_typekind	res;
	t_token		*ident;
	t_type		*type;

	switch (g_token->kind)
	{
		case TK_INT:
			res = TY_INT;
			break ;
		case TK_CHAR:
			res = TY_CHAR;
			break ;
		case TK_BOOL:
			res = TY_BOOL;
			break ;
		case TK_VOID:
			res = TY_VOID;
			break ;
		case TK_FLOAT:
			res = TY_FLOAT;
			break ;
		case TK_STRUCT:
		{
			consume_any();
			ident = consume_ident();
			if (ident == NULL)
				return (NULL);
			if (consume("{"))
				read_struct_block(ident);
			type = new_struct_type(ident->str, ident->len);
			if (type == NULL)
				return (NULL);
			return (type);
		}
		case TK_ENUM:
		{
			consume_any();
			ident = consume_ident();
			if (ident == NULL)
				return (NULL);
			if (consume("{"))
				read_enum_block(ident);
			type = new_enum_type(ident->str, ident->len);
			if (type == NULL)
				return (NULL);
			return (type);
		}
		case TK_UNION:
		{
			consume_any();
			ident = consume_ident();
			if (ident == NULL)
				return (NULL);
			if (consume("{"))
				read_union_block(ident);
			type = new_union_type(ident->str, ident->len);
			if (type == NULL)
				return (NULL);
			return (type);
		}
		default:
		{
			if (consume_type_alias(&type))
				return (type);
			return (NULL);
		}
	}
	consume_any();
	type = new_primitive_type(res);
	return (type);
}

t_type	*consume_type_before(void)
{
	t_type	*type;
	bool	is_unsigned;

	is_unsigned = false;
	while (consume_kind(TK_UNSIGNED))
		is_unsigned = true;

	// type name
	type = consume_type_specifier();
	if (type == NULL)
		return (NULL);

	// TODO unsignedを許容するかはとりあえず無視
	type->is_unsigned = is_unsigned;

	consume_type_ptr(&type);
	return type;
}

// 型宣言のarray部分を読む
void	expect_type_after(t_type **type)// expect size
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

bool	consume_enum_key(t_type **type, int *value)
{
	t_token	*tok;
	t_defenum	*res_def;

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

int	expect_number(void)
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
