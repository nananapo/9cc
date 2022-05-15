#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern StructDef	*struct_defs[];

static int max(int a, int b)
{
	return a > b ? a : b;
}

Type	*new_primitive_type(PrimitiveType pri)
{
	Type	*type = calloc(1, sizeof(Type));
	type->ty = pri;
	type->ptr_to = NULL;
	type->next = NULL;
	return type;
}

Type	*new_type_ptr_to(Type *ptr_to)
{
	Type	*type = new_primitive_type(PTR);
	type->ptr_to = ptr_to;
	return type;
}

Type	*new_type_array(Type *ptr_to)
{
	Type	*type = new_primitive_type(ARRAY);
	type->ptr_to = ptr_to;
	return type;
}

Type	*new_struct_type(char *name, int len)
{
	Type	*type;
	int		i;

	type = new_primitive_type(STRUCT);
	for (i = 0; struct_defs[i]; i++)
	{
		if (struct_defs[i]->name_len == len
		&& strncmp(struct_defs[i]->name, name, len) == 0)
		{
			type->strct = struct_defs[i];
			break ;
		}
	}
	if (type->strct == NULL)
		return (NULL);
	return (type);
}

// 2つのTypeが一致するかどうか
bool	type_equal(Type *t1, Type *t2)
{
	if (t1->ty != t2->ty)
	{
		if ((t1->ty == PTR && t2->ty == ARRAY)
			|| (t1->ty == ARRAY && t2->ty == PTR))
			return type_equal(t1->ptr_to, t2->ptr_to);
		return false;
	}
	if (t1->ty == PTR)
		return type_equal(t1->ptr_to, t2->ptr_to);
	if (t1->ty == ARRAY)
		return type_equal(t1->ptr_to, t2->ptr_to);
	if (t1->ty == STRUCT)
		return (t1->strct == t2->strct);
	return true;
}

// typeのサイズを取得する
int	type_size(Type *type)
{
	if (type->ty == INT)
		return (4);
	if (type->ty == CHAR)
		return (1);
	if (type->ty == PTR)
		return (8);
	if (type->ty == ARRAY)
	{
		/*
		if (type->ptr_to-> == PTR || type->ptr_to->ty == ARRAY)
			return (type_size(type->ptr_to) * type->array_size);
		if (is_integer_type(type->ptr_to))
			return (type_size(type->ptr_to) * type->array_size);
		*/
		return (type_size(type->ptr_to) * type->array_size);
	}
	if (type->ty == STRUCT)
		return (align_to(type->strct->mem_size, 8));
	return -1;
}

// 整数型かどうか判定する
bool	is_integer_type(Type *type)
{
	return (type->ty == INT
			|| type->ty == CHAR);
}

// 配列かどうか確認する
bool	is_pointer_type(Type *type)
{
	return (type->ty == ARRAY
			|| type->ty == PTR);
}

// 比較可能か調べる
bool	can_compared(Type *l, Type *r)
{
	if (type_equal(l, r))
		return (true);
	if (is_integer_type(l) && is_integer_type(r))
		return (true);
	if (is_pointer_type(l) && is_pointer_type(r))
		return (true);
	return (false);
}

// lにrを代入可能か確かめる
bool	can_assign(Type *l, Type *r)
{
	if (type_equal(l, r))
		return (true);
	if (is_integer_type(l) && is_integer_type(r))
		return (true);
	return (false);
}

char	*type_regname(Type *type)
{
	// アドレスは8byte
	if (type->ty == PTR || type->ty == ARRAY)
		return RAX;
	// INTは4byte
	if (type->ty == INT)
		return EAX;
	// CHARは1byte
	if (type->ty == CHAR)
		return AL;
	printf("不明な型 %d\n", type->ty);
	exit(1);
	return NULL;
}

StructMemberElem	*struct_get_member(StructDef *strct, char *name, int len)
{
	if (strct == NULL)
		return (NULL);
	for (StructMemberElem *mem = strct->members; mem != NULL; mem = mem->next)
	{
		if (mem->name_len == len && strncmp(name, mem->name, len) == 0)
			return (mem);
	}
	return (NULL);
}
