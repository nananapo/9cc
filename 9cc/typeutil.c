#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

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

// 2つのTypeが一致するかどうか
bool	type_equal(Type *t1, Type *t2)
{
	if (t1->ty != t2->ty)
	{
		if ((t1->ty == PTR && t2->ty == ARRAY)
			|| (t1->ty == ARRAY && t2->ty == PTR))
			return type_equal(t1->ptr_to, t2->ptr_to); // TODO とりあえず
		return false;
	}
	if (t1->ty == PTR)
		return type_equal(t1->ptr_to, t2->ptr_to);
	if (t1->ty == ARRAY)
		return type_equal(t1->ptr_to, t2->ptr_to);
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
		return type_size(type->ptr_to) * type->array_size;
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
