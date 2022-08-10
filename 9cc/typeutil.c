#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mymath.h"

// main
extern t_token			*g_token;
extern t_deffunc		*g_func_defs[1000];
extern t_deffunc		*g_func_protos[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals[1000];
extern t_defstruct		*g_struct_defs[1000];
extern t_defenum		*g_enum_defs[1000];
extern t_defunion		*g_union_defs[1000];
extern t_lvar			*g_locals;
extern t_deffunc		*g_func_now;
extern t_linked_list	*g_type_alias;

bool	find_enum(char *str, int len, t_defenum **res_def, int *res_value)
{
	int	i;
	int	j;

	for (i = 0; g_enum_defs[i]; i++)
	{
		t_defenum	*def = g_enum_defs[i];
		for (j = 0; j < def->kind_len; j++)
		{
			char *var = def->kinds[j];
			if ((int)strlen(var) == len
			&& strncmp(str, var, strlen(var)) == 0)
			{
				if (res_def != NULL)
					*res_def = def;
				if (res_value != NULL)
					*res_value = j;
				return (true);
			}
		}
	}
	return (false);
}

t_type	*new_primitive_type(t_typekind pri)
{
	t_type	*type = calloc(1, sizeof(t_type));
	type->ty = pri;
	type->ptr_to = NULL;
	return type;
}

t_type	*new_type_ptr_to(t_type *ptr_to)
{
	t_type	*type = new_primitive_type(TY_PTR);
	type->ptr_to = ptr_to;
	return type;
}

t_type	*new_type_array(t_type *ptr_to)
{
	t_type	*type = new_primitive_type(TY_ARRAY);
	type->ptr_to = ptr_to;
	return type;
}

t_type	*new_enum_type(char *name, int len)
{
	t_type	*type;
	int		i;

	type = new_primitive_type(TY_ENUM);
	for (i = 0; g_enum_defs[i]; i++)
	{
		if (g_enum_defs[i]->name_len == len
		&& strncmp(g_enum_defs[i]->name, name, len) == 0)
		{
			type->enm = g_enum_defs[i];
			break ;
		}
	}
	if (type->enm == NULL)
		return (NULL);
	return (type);
}

t_type	*new_struct_type(char *name, int len)
{
	t_type	*type;
	int		i;

	type = new_primitive_type(TY_STRUCT);
	for (i = 0; g_struct_defs[i]; i++)
	{
		if (g_struct_defs[i]->name_len == len
		&& strncmp(g_struct_defs[i]->name, name, len) == 0)
		{
			type->strct = g_struct_defs[i];
			break ;
		}
	}
	if (type->strct == NULL)
		return (NULL);
	return (type);
}

t_type	*new_union_type(char *name, int len)
{
	t_type	*type;
	int		i;

	type = new_primitive_type(TY_UNION);
	for (i = 0; g_union_defs[i]; i++)
	{
		if (g_union_defs[i]->name_len == len
		&& strncmp(g_union_defs[i]->name, name, len) == 0)
		{
			type->unon = g_union_defs[i];
			break ;
		}
	}
	if (type->unon == NULL)
		return (NULL);
	return (type);
}

// 2つのt_typeが一致するかどうか
bool	type_equal(t_type *t1, t_type *t2)
{
	if (t1->ty != t2->ty)
	{
		if ((t1->ty == TY_PTR && t2->ty == TY_ARRAY)
			|| (t1->ty == TY_ARRAY && t2->ty == TY_PTR))
			return (type_equal(t1->ptr_to, t2->ptr_to));
		return (false);
	}
	if (t1->ty == TY_PTR)
		return type_equal(t1->ptr_to, t2->ptr_to);
	if (t1->ty == TY_ARRAY)
		return type_equal(t1->ptr_to, t2->ptr_to);
	if (t1->ty == TY_STRUCT)
		return (t1->strct == t2->strct);
	if (t2->ty == TY_UNION)
		return (t1->unon == t2->unon);
	if (t1->ty == TY_ENUM)
		return (t1->enm == t2->enm);
	return (true);
}

// 整数型かどうか判定する
bool	is_integer_type(t_type *type)
{
	return (type->ty == TY_INT
			|| type->ty == TY_CHAR
			|| type->ty == TY_ENUM
			|| type->ty == TY_BOOL);
}

// 配列かどうか確認する
bool	is_pointer_type(t_type *type)
{
	return (type->ty == TY_ARRAY
			|| type->ty == TY_PTR);
}

// 比較可能か調べる
bool	can_compared(t_type *l, t_type *r, t_type **lt, t_type **rt)
{
	if (l->ty == TY_VOID || r->ty == TY_VOID)
		return (false);

	*lt = l;
	*rt = r;

	if (type_equal(l, r))
		return (true);
	if (is_pointer_type(l) && is_pointer_type(r))
		return (true);

	if (is_integer_type(l) && is_integer_type(r))
	{
		// とりあえずintにする
		*lt = new_primitive_type(TY_INT);
		*rt = new_primitive_type(TY_INT);
		return (true);
	}

	if ((is_pointer_type(l) && is_integer_type(r))
	|| (is_integer_type(l) && is_pointer_type(r)))
	{
		*lt = new_type_ptr_to(new_primitive_type(TY_VOID));
		*rt = new_type_ptr_to(new_primitive_type(TY_VOID));
		return (true);
	}

	return (false);
}

t_member	*struct_get_member(t_defstruct *strct, char *name, int len)
{
	t_member	*mem;

	if (strct == NULL)
		return (NULL);
	for (mem = strct->members; mem != NULL; mem = mem->next)
	{
		if (mem->name_len == len && strncmp(name, mem->name, len) == 0)
			return (mem);
	}
	return (NULL);
}

// struct_get_memberのunion版
t_member	*union_get_member(t_defunion *strct, char *name, int len)
{
	t_member	*mem;

	if (strct == NULL)
		return (NULL);
	for (mem = strct->members; mem != NULL; mem = mem->next)
	{
		if (mem->name_len == len && strncmp(name, mem->name, len) == 0)
			return (mem);
	}
	return (NULL);
}

// structかunionの時にstruct_get_memberかunion_get_memberを呼ぶ
t_member	*get_member_by_name(t_type *type, char *name, int len)
{
	if (type->ty == TY_STRUCT)
		return (struct_get_member(type->strct, name, len));
	if (type->ty == TY_UNION)
		return (union_get_member(type->unon, name, len));
	return (NULL);
}

static void	typename_loop(t_type *type, char *str)
{
	if (type->ty == TY_INT)
		strcat(str, "int");
	else if (type->ty == TY_CHAR)
		strcat(str, "char");
	else if (type->ty == TY_BOOL)
		strcat(str, "_Bool");
	else if (type->ty == TY_VOID)
		strcat(str, "void");
	else if (type->ty == TY_STRUCT)
	{
		strcat(str, "struct[");
		strncat(str, type->strct->name, type->strct->name_len);
		strcat(str, "]");
	}
	else if (type->ty == TY_ENUM)
	{
		strcat(str, "enum");
		strncat(str, type->enm->name, type->enm->name_len);
		strcat(str, "]");
	}
	else if (type->ty == TY_UNION)
	{
		strcat(str, "union");
		strncat(str, type->unon->name, type->unon->name_len);
		strcat(str, "]");
	}
	else if (type->ty == TY_ARRAY)
	{
		typename_loop(type->ptr_to, str);
		strcat(str, "[]");
	}
	else if (type->ty == TY_PTR)
	{
		typename_loop(type->ptr_to, str);
		strcat(str, "*");
	}
	else
	{
		error("未対応の型 %d", type->ty);
	}
}

// 宣言可能な型かを確かめる
// Voidか確かめるだけ
bool	is_declarable_type(t_type *type)
{
	return (type->ty != TY_VOID);
}

bool	type_can_cast(t_type *from, t_type *to, bool is_explicit)
{
	if (type_equal(from, to))
		return (true);

	// structはダメ
	if (from->ty == TY_STRUCT || to->ty == TY_STRUCT)
		return (false);

	// unionもダメ
	if (from->ty == TY_UNION || to->ty == TY_UNION)
		return (false);

	// どちらもポインタ
	if (is_pointer_type(from) && is_pointer_type(to))
		return (true);

	// どちらかがポインタ
	if (is_pointer_type(from) != is_pointer_type(to))
		return (true);

	// どちらも数字？
	if (is_explicit)
		debug("");
	is_explicit = false;
	return (true);
}

char	*get_type_name(t_type *type)
{
	char	*ret;

	if (type == NULL)
		fprintf(stderr, "typeがNULLです");
	ret = calloc(1000, sizeof(char));
	typename_loop(type, ret);
	return ret;
}

t_type	*type_array_to_ptr(t_type *type)
{
	if (type->ty != TY_ARRAY)
		return (type);
	return (new_type_ptr_to(type->ptr_to));
}

bool	can_use_arrow(t_type *type)
{
	return (is_pointer_type(type) && can_use_dot(type->ptr_to));
}

bool	can_use_dot(t_type *type)
{
	return (type->ty == TY_STRUCT || type->ty == TY_UNION);
}
