#include "9cc.h"
#include "il.h"
#include "charutil.h"
#include "mymath.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// main
extern t_deffunc		*g_func_defs[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals[1000];
extern t_il				*g_il;

int	get_array_align_size_verilog(t_type *type)
{
	return (get_type_size(type));
}

int	get_type_size_verilog(t_type *type)
{
	switch (type->ty)
	{
		case TY_INT:
			return (4);
		case TY_CHAR:
			return (1);
		case TY_BOOL:
			return (1);
		case TY_FLOAT:
			return (4);
		case TY_DOUBLE:
			return (4);
		case TY_PTR:
			return (8);
		case TY_ARRAY:
			return (get_type_size(type->ptr_to) * type->array_size);
		case TY_VOID:
			return (1);
		case TY_ENUM:
			return  (4);
		case TY_STRUCT:
			return (get_struct_size(type));
		case TY_UNION:
			return (get_union_size(type->unon));
		default:
			error("size of unknown type %d", type->ty);
	}
	return (-1);
}

bool	is_unsigned_abi_verilog(t_typekind kind)
{
	switch (kind)
	{
		case TY_PTR:
		case TY_ARRAY:
			return (true);
		default:
			return (false);
	}
	return (false);
}

static void	gen_il(t_il *code)
{
	switch (code->kind)
	{
		default:
		{
			fprintf(stderr, "gen not implemented\nkind : %d\n", code->kind);
			error("Error");
			return ;
		}
	}
}

void	codegen_verilog(void)
{
	int		i;
	t_il	*code;
	for (code = g_il; code != NULL; code = code->next)
		gen_il(code);
}
