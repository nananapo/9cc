#include "9cc.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

extern LVar		*locals;
extern Node		*global_vars[];

int	align_to(int n, int align);
int	max(int a, int b);

LVar	*find_lvar(char *str, int len)
{
	for (LVar *var = locals; var; var = var->next)
		if (var->len == len && memcmp(str, var->name, var->len) == 0)
			return var;
	return NULL;
}

Node	*find_global(char *str, int len)
{
	for (int i = 0; global_vars[i]; i++)
		if (len == global_vars[i]->var_name_len
			&& memcmp(str,
					global_vars[i]->var_name,
					global_vars[i]->var_name_len) == 0)
			return global_vars[i];
	return NULL;
}

// TODO check same structも
// TODO struct
int	create_local_var(char *name, int len, Type *type, bool is_arg)
{
	LVar	*lvar;
	int		size;

	lvar = calloc(1, sizeof(LVar));
	lvar->next = locals;
	lvar->name = name;
	lvar->len = len;
	lvar->type = type;

	size = type_size(type);

	if (locals == NULL)
	{
		if (is_arg)
			lvar->offset = max(8, size);
		else
			lvar->offset = size;
	}
	else
	{
		if (is_arg)
			lvar->offset = locals->offset + max(8, size);
		else
		{
			if (size < 4)
			{
				if (locals->offset % 4 + size <= 4)
					lvar->offset = locals->offset + size;
				else
					lvar->offset = (locals->offset + 3) / 4 * 4 + size;
			}
			else if (size == 4)
				lvar->offset = (locals->offset + 3) / 4 * 4 + size;
			else
				lvar->offset = (locals->offset + 7) / 8 * 8 + size;
		}
	}
	locals = lvar;
	return lvar->offset;
}
