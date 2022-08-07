#include "9cc.h"
#include "parse.h"
#include "mymath.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// main
extern t_deffunc		*g_func_defs[1000];
extern t_deffunc		*g_func_protos[1000];
extern t_defvar			*g_global_vars[1000];

// TODO スコープ
t_lvar	*find_lvar(t_deffunc *func, char *str, int len)
{
	t_lvar	*var;

	for (var = func->locals; var; var = var->next)
		if (!var->is_dummy
		&& var->name != NULL
		&& var->name_len != 0
		&& var->name_len == len
		&& memcmp(str, var->name, var->name_len) == 0)
			return var;
	return NULL;
}

t_defvar	*find_global(char *str, int len)
{
	int	i;

	for (i = 0; g_global_vars[i]; i++)
		if (len == g_global_vars[i]->name_len
			&& memcmp(str,
					g_global_vars[i]->name,
					g_global_vars[i]->name_len) == 0)
			return g_global_vars[i];
	return NULL;
}

t_lvar	*append_lvar(t_deffunc *func, char *name, int name_len, t_type *type, bool is_argument)
{
	t_lvar	*lvar;
	t_lvar	*tmp;

	lvar				= calloc(1, sizeof(t_lvar));
	lvar->name			= name;
	lvar->name_len		= name_len;
	lvar->type			= type;
	lvar->is_argument	= is_argument;

	// append
	if (func->locals == NULL)
		func->locals = lvar;
	else
	{
		for(tmp = func->locals; tmp->next != NULL; tmp = tmp->next);
		tmp->next = lvar;
	}
	return (lvar);
}

t_lvar	*append_dummy_lvar(t_deffunc *func, t_type *type)
{
	t_lvar	*lvar;

	lvar			= append_lvar(func, "", 0, type, false);
	lvar->is_dummy	= true;
	return (lvar);
}
