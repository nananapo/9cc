#include "9cc.h"
#include <stdlib.h>
#include <string.h>

extern LVar		*locals;

int	align_to(int n, int align);

LVar	*find_lvar(char *str, int len)
{
	for (LVar *var = locals; var; var = var->next)
		if (var->len == len && memcmp(str, var->name, var->len) == 0)
			return var;
	return NULL;
}

int	create_local_var(char *name, int len, Type *type)
{
	// TODO check same
	LVar *lvar = calloc(1, sizeof(LVar));
	lvar->next = locals;
	lvar->name = name;
	lvar->len = len;
	lvar->type = type;
	if (locals == NULL)
		lvar->offset = align_to(type_size(type), 8);
	else
		lvar->offset = align_to(locals->offset + type_size(type), 8);
	locals = lvar;
	return lvar->offset;
}

int	get_locals_count()
{
	int	i = 0;
	LVar *tmp = locals;
	while (tmp)
	{
		i++;
		tmp = tmp->next;
	}
	return i;
}
