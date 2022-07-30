#include "9cc.h"
#include "parse.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

int	align_to(int n, int align);
int	max(int a, int b);
int	min(int a, int b);

static void alloc_local_var(LVar *lvar);
Type	*type_cast_forarg(Type *type);

// main
extern Token			*g_token;
extern t_deffunc		*g_func_defs[1000];
extern t_deffunc		*g_func_protos[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals;
extern StructDef		*g_struct_defs[1000];
extern EnumDef			*g_enum_defs[1000];
extern UnionDef			*g_union_defs[1000];
extern LVar				*g_locals;
extern t_deffunc		*g_func_now;
extern t_linked_list	*g_type_alias;


bool	find_enum(char *str, int len, EnumDef **res_def, int *res_value)
{
	int	i;
	int	j;

	for (i = 0; g_enum_defs[i]; i++)
	{
		EnumDef	*def = g_enum_defs[i];
		for (j = 0; j < def->kind_len; j++)
		{
			char *var = def->kinds[j];
			if ((int)strlen(var) == len
			&& strncmp(str, var, strlen(var)) == 0)
			{
				if (res_def != NULL)
					*res_def = def;
				if (res_value != NULL)
					*res_value = j;
				return (true);
			}
		}
	}
	return (false);
}

LVar	*find_lvar(char *str, int len)
{
	LVar	*var;

	for (var = g_locals; var; var = var->next)
		if (!var->is_dummy && var->len == len && memcmp(str, var->name, var->len) == 0)
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

LVar	*copy_lvar(LVar *f)
{
	LVar	*lvar;

	lvar = calloc(1, sizeof(LVar));
	*lvar = *f;
	return (lvar);
}

void	alloc_argument_simu(LVar *first, LVar *lvar)
{
	int		size;
	int		regindex_max;
	int		offset_min;
	int		offset_max;
	LVar	*tmp;

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
		// alloc_local_var(lvar);
		return ;
	}

	// スタックに入れる
	lvar->offset = offset_min;
}



// TODO サイズ0はどうなるか確かめる
static void	alloc_argument(LVar *lvar)
{
	alloc_argument_simu(g_locals, lvar);
}

static void alloc_local_var(LVar *lvar)
{
	int		size;
	int 	offset_max;
	LVar	*tmp;

	// 引数のオフセットの正の最大値を求める
	offset_max = 0;
	for (tmp = g_locals; tmp; tmp = tmp->next)
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
// TODO struct
LVar	*create_local_var(char *name, int len, Type *type, bool is_arg)
{
	LVar	*lvar;
	LVar	*tmp;

	lvar = calloc(1, sizeof(LVar));
	lvar->name = name;
	lvar->len = len;
	if (is_arg)
		type = type_cast_forarg(type);
	lvar->type = type;
	lvar->is_arg = is_arg;
	lvar->arg_regindex = -1;
	lvar->is_dummy = false;

	// メモリを割り当て
	if (is_arg)
		alloc_argument(lvar);
	else
		alloc_local_var(lvar);

	// localsに保存
	lvar->next = NULL;
	if (g_locals == NULL)
		g_locals = lvar;
	else
	{
		for(tmp = g_locals; tmp; tmp = tmp->next)
		{
			if (tmp->next == NULL)
			{
				tmp->next = lvar;
				break ;
			}
		}
	}
	debug("CREATED LVAR %s", strndup(name, len));
	return lvar;
}
