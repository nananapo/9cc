#include "9cc.h"
#include "parse.h"
#include "mymath.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// main
extern t_token			*g_token;
extern t_deffunc		*g_func_defs[1000];
extern t_deffunc		*g_func_protos[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals;
extern t_defstruct		*g_struct_defs[1000];
extern t_defenum		*g_enum_defs[1000];
extern t_defunion		*g_union_defs[1000];
extern t_deffunc		*g_func_now;
extern t_linked_list	*g_type_alias;

t_lvar	*find_lvar(t_deffunc *func, char *str, int len)
{
	t_lvar	*var;

	for (var = func->locals; var; var = var->next)
		if (!var->is_dummy && var->name_len == len
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

t_lvar	*copy_lvar(t_lvar *f)
{
	t_lvar	*lvar;

	lvar = calloc(1, sizeof(t_lvar));
	*lvar = *f;
	return (lvar);
}

void	alloc_argument_simu(t_lvar *first, t_lvar *lvar)
{
	int		size;
	int		regindex_max;
	int		offset_min;
	int		offset_max;
	t_lvar	*tmp;

	regindex_max = -1;
	// rbpをプッシュした分を考慮する
	offset_min = -16;
	offset_max = 0;
	for (tmp = first; tmp; tmp = tmp->next)
	{
		if (!tmp->is_arg)
			continue ;

		regindex_max = max(regindex_max, tmp->arg_regindex);
		if (tmp->arg_regindex == -1)
		{
			offset_min = min(offset_min, tmp->offset - align_to(get_type_size(tmp->type), 8));
		}
		offset_max = max(offset_max, max(0, tmp->offset));
	}

	size = get_type_size(type_array_to_ptr(lvar->type)); // 配列はポインタにする

	// レジスタに入れるか決定する
	if (regindex_max < ARGREG_SIZE - 1
	&& (ARGREG_SIZE - regindex_max - 1) * 8 >= size)
	{
		// とりあえず必ず8byteにする
		lvar->arg_regindex = regindex_max + align_to(size, 8) / 8;
		lvar->offset = (offset_max + size + 7) / 8 * 8;
		return ;
	}

	// スタックに入れる
	lvar->offset = offset_min;
}



// TODO サイズ0はどうなるか確かめる
static void	alloc_argument(t_deffunc *func, t_lvar *lvar)
{
	alloc_argument_simu(func->locals, lvar);
}

static void alloc_local_var(t_deffunc *func, t_lvar *lvar)
{
	int		size;
	int 	offset_max;
	t_lvar	*tmp;

	// 引数のオフセットの正の最大値を求める
	offset_max = 0;
	for (tmp = func->locals; tmp != NULL; tmp = tmp->next)
	{
		offset_max = max(offset_max, max(0, tmp->offset));
	}
	size = get_type_size(lvar->type);

	// offsetの最大値を基準に配置する
	if (size < 4)
	{
		if (offset_max % 4 + size <= 4)
			lvar->offset = offset_max + size;
		else
			lvar->offset = (offset_max + 3) / 4 * 4 + size;
	}
	else if (size == 4)
		lvar->offset = (offset_max + 3) / 4 * 4 + size;
	else
		lvar->offset = (offset_max + 7) / 8 * 8 + size;
}

// TODO 同じ名前の変数がないかチェックする
t_lvar	*create_lvar(t_deffunc *func, char *name, int name_len, t_type *type, bool is_arg)
{
	t_lvar	*lvar;
	t_lvar	*tmp;

	lvar				= calloc(1, sizeof(t_lvar));
	lvar->name			= name;
	lvar->name_len		= name_len;
	lvar->type			= type;
	lvar->is_arg		= is_arg;
	lvar->arg_regindex	= -1;
	lvar->is_dummy		= false;
	lvar->next			= NULL;

	// スタックを割り当て
	if (is_arg)
		alloc_argument(func, lvar);
	else
		alloc_local_var(func, lvar);

	// append
	if (func->locals == NULL)
		func->locals = lvar;
	else
	{
		for(tmp = func->locals; tmp->next != NULL; tmp = tmp->next);
		tmp->next = lvar;
	}
	debug("lvar: %s created", strndup(name, name_len));
	return lvar;
}
