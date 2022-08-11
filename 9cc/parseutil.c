#include "9cc.h"
#include "parse.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

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

bool consume_float(float *result)
{
	if (g_token->kind != TK_FLOAT)
		return false;
	*result = g_token->val_float;
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

static void	consume_array(t_type **type)
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
		case TK_DOUBLE:
			res = TY_DOUBLE;
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

static t_type *expect_function(t_token *name, bool is_static)
{
	t_deffunc	*def;
	t_token		*ident;
	t_type		*type;

	def					= calloc(1, sizeof(t_deffunc));
	def->name			= name->str;
	def->name_len		= name->len;
	def->argcount		= 0;
	def->is_static		= is_static;

	//printf("exp %s\n", g_token->str);

	// 引数を読む
	if (!consume(")"))
	{
		for (;;)
		{
			// variable argument
			if (consume("..."))
			{
				if (def->argument_types[0] == NULL)
					error_at(g_token->str, "可変長引数の宣言をするには、少なくとも一つの引数が必要です");
				if (!consume(")"))
					error_at(g_token->str, ")が必要です");
				def->is_variable_argument = true;
				break ;
			}

			// 型宣言の確認
			if (!consume_type(&type, &ident, NULL, NULL))
				error_at(g_token->str,"宣言が必要です\n (funcdef)");
			if (ident == NULL)
			{
				// voidなら引数0個
				if (type->ty == TY_VOID)
				{
					if (def->argument_types[0] != NULL)
						error_at(g_token->str, "既に引数が宣言されています");
					if (!consume(")"))
						error_at(g_token->str, ")が見つかりませんでした。");
					def->is_zero_argument = true;
					break ;
				}
				error_at(g_token->str, "仮引数が必要です");
			}
			type = type_array_to_ptr(type);

			// save
			def->argument_names[def->argcount]		= ident->str;
			def->argument_name_lens[def->argcount]	= ident->len;
			def->argument_types[def->argcount]		= type;
			def->argcount += 1;

			// )か,
			if (consume(")"))
				break;
			if (!consume(","))
				error_at(g_token->str, ",が必要です (ef)");
		}
	}

	type			= new_primitive_type(TY_FUNCTION);
	type->funcdef	= (void *)def;

	//printf("dexp %s\n", g_token->str);

	return (type);
}

bool	consume_type(t_type **r_type, t_token **r_ident, bool *is_static, bool *is_extern)
{
	t_type	*type;
	t_type	*functype;
	t_token	*ident;
	int		pointer_count;
	bool	__iss;
	bool	__ise;

	if (is_static == NULL)
		is_static = &__iss;
	if (is_extern == NULL)
		is_extern = &__ise;

	if (consume_kind(TK_STATIC))
			*is_static = true;
	else
			*is_static = false;

	if (consume_kind(TK_EXTERN))
			*is_extern = true;
	else
			*is_extern = false;

	if ((type = consume_type_specifier()) == NULL)
		return (false);

	while (consume("*"))
		type = new_type_ptr_to(type);

	ident = NULL;
	if (consume("("))
	{
		pointer_count = 0;
		while (consume("*"))
			pointer_count += 1;
		ident = consume_ident();
		if (!consume(")"))
			error_at(g_token->str, "not func 0");
		if (!consume("("))
			error_at(g_token->str, "not func 1");
		functype = expect_function(ident, *is_static);
		if (functype == NULL)
			error_at(g_token->str, "functype is NULL 1");
		((t_deffunc *)functype->funcdef)->type_return = type;
		for (; pointer_count > 0; pointer_count--)
			functype = new_type_ptr_to(functype);
		*r_ident = ident;
		*r_type = functype;
		return (true);
	}

	consume_array(&type);

	ident = consume_ident();
	if (consume("("))
	{
		functype = expect_function(ident, *is_static);
		if (functype == NULL)
			error_at(g_token->str, "functype is NULL 2");
		((t_deffunc *)functype->funcdef)->type_return = type;
		*r_ident = ident;
		*r_type = functype;
		return (true);
	}

	consume_array(&type);

	*r_type	= type;
	*r_ident= ident;
	return (true);
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
