#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

static int max(int a, int b)
{
	if (a > b)
		return (a);
	return (b);
}

Type	*new_primitive_type(PrimitiveType pri)
{
	Type	*type = calloc(1, sizeof(Type));
	type->ty = pri;
	type->ptr_to = NULL;
	return type;
}

Type	*new_type_ptr_to(Type *ptr_to)
{
	Type	*type = new_primitive_type(TY_PTR);
	type->ptr_to = ptr_to;
	return type;
}

Type	*new_type_array(Type *ptr_to)
{
	Type	*type = new_primitive_type(TY_ARRAY);
	type->ptr_to = ptr_to;
	return type;
}

Type	*new_enum_type(char *name, int len)
{
	Type	*type;
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

Type	*new_struct_type(char *name, int len)
{
	Type	*type;
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

Type	*new_union_type(char *name, int len)
{
	Type	*type;
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

// 2つのTypeが一致するかどうか
bool	type_equal(Type *t1, Type *t2)
{
	if (t1->ty != t2->ty)
	{
		if ((t1->ty == TY_PTR && t2->ty == TY_ARRAY)
			|| (t1->ty == TY_ARRAY && t2->ty == TY_PTR))
			return type_equal(t1->ptr_to, t2->ptr_to);
		return false;
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
	return true;
}

// typeのサイズを取得する
int	get_type_size(Type *type)
{
	if (type->ty == TY_INT)
		return (4);
	if (type->ty == TY_CHAR)
		return (1);
	if (type->ty == TY_BOOL)
		return (1);
	if (type->ty == TY_PTR)
		return (8);
	if (type->ty == TY_ARRAY)
		return (get_type_size(type->ptr_to) * type->array_size);
	if (type->ty == TY_STRUCT)
		return (type->strct->mem_size);
	if (type->ty == TY_ENUM)
		return (4);
	if (type->ty == TY_UNION)
		return (type->unon->mem_size);
	if (type->ty == TY_VOID)
		return (1);
	return -1;
}

// 整数型かどうか判定する
bool	is_integer_type(Type *type)
{
	return (type->ty == TY_INT
			|| type->ty == TY_CHAR
			|| type->ty == TY_ENUM
			|| type->ty == TY_BOOL);
}

// 配列かどうか確認する
bool	is_pointer_type(Type *type)
{
	return (type->ty == TY_ARRAY
			|| type->ty == TY_PTR);
}

// 比較可能か調べる
bool	can_compared(Type *l, Type *r, Type **lt, Type **rt)
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

MemberElem	*struct_get_member(StructDef *strct, char *name, int len)
{
	MemberElem	*mem;

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
MemberElem	*union_get_member(UnionDef *strct, char *name, int len)
{
	MemberElem	*mem;

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
MemberElem	*get_member_by_name(Type *type, char *name, int len)
{
	if (type->ty == TY_STRUCT)
		return (struct_get_member(type->strct, name, len));
	if (type->ty == TY_UNION)
		return (union_get_member(type->unon, name, len));
	return (NULL);
}

int	max_type_size(Type *type)
{
	MemberElem	*tmp;
	int			size;

	if (type->ty == TY_STRUCT)
	{
		size = 0;
		for (tmp = type->strct->members; tmp; tmp = tmp->next)
		{
			size = max(size, max_type_size(tmp->type));
		}
		return (size);
	}
	else if (type->ty == TY_ARRAY)
	{
		return (max_type_size(type->ptr_to));
	}
	return get_type_size(type);
}

static void	typename_loop(Type *type, char *str)
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
		strcat(str, "struct"); // TODO struct name
	else if (type->ty == TY_ENUM)
		strcat(str, "enum");
	else if (type->ty == TY_UNION)
		strcat(str, "union");
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
bool	is_declarable_type(Type *type)
{
	return (type->ty != TY_VOID);
}

bool	type_can_cast(Type *from, Type *to, bool is_explicit)
{
	int	size1;
	int	size2;

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
	size1 = get_type_size(from);
	size2 = get_type_size(to);

	is_explicit = false;

	return (true);
}

Type	*type_cast_forarg(Type *type)
{
	if (type->ty == TY_PTR)
		return (new_type_ptr_to(type->ptr_to));
	else
		return (type);
}

char	*get_type_name(Type *type)
{
	char	*ret;

	if (type == NULL)
		fprintf(stderr, "typeがNULLです");
	ret = calloc(1000, sizeof(char));
	typename_loop(type, ret);
	return ret;
}

Type	*type_array_to_ptr(Type *type)
{
	if (type->ty != TY_ARRAY)
		return (type);
	return (new_type_ptr_to(type->ptr_to));
}

bool	can_use_arrow(Type *type)
{
	return (is_pointer_type(type) && can_use_dot(type->ptr_to));
}

bool	can_use_dot(Type *type)
{
	return (type->ty == TY_STRUCT || type->ty == TY_UNION);
}

bool	is_memory_type(Type *type)
{
	if (type->ty != TY_STRUCT)
		return (false);
	return (get_type_size(type) > 16);
}
