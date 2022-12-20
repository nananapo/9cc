#include "ir.h"
#include "regalloc.h"
#include <stdlib.h>
#include <stdbool.h>

extern t_ir_func		*g_ir_funcs[1000];

void	allocate_register(void)
{
	int	i;

	for (i = 0; g_ir_funcs[i] != NULL; i++)
	{
		allocate_func(g_ir_funcs[i]->func->);
	}
}
