#include "9cc.h"
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
int	type_size(Type *type, int min)
{
	if (type->ty == INT)
		return max(4, min);
	if (type->ty == PTR)
		return max(8, min);
	if (type->ty == ARRAY)
		return max(min, type_size(type->ptr_to, min) * type->array_size);
	return -1;
}
