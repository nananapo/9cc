#include "9cc.h"

int	get_array_align_size_riscv(t_type *type)
{
	return (get_type_size(type));
}

int	get_type_size_riscv(t_type *type)
{
	switch (type->ty)
	{
		case TY_INT:
			return (4);
		case TY_CHAR:
			return (1);
		case TY_BOOL:
			return (1);
		case TY_PTR:
			return (8);
		case TY_ARRAY:
			return (get_type_size(type->ptr_to) * type->array_size);
		case TY_VOID:
			return (1);
		case TY_ENUM:
			return  (4);
		case TY_STRUCT:
	//		return (get_struct_size(type));
		case TY_UNION:
	//		return (get_union_size(type->unon));
		default:
			error("size of unknown type %d", type->ty);
	}
	return (-1);
}

void	codegen_riscv(void)
{
}
