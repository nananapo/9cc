#include "ir.h"
#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

extern t_ir_func		*g_ir_funcs[1000];
extern t_deffunc		*g_func_defs[1000];

static t_ir_stmt_base	*g_ir;
static t_ir_stmt_base	*g_ir_last;

static t_ir	*malloc_ir(t_irkind kind);
static void	append_stmt(t_ir_stmt_base *ir);
static void	translate_stmt(t_node *node);
static void	translate_return(t_node *node);
static void	translate_block(t_node *node);


static t_ir	*malloc_ir(t_irkind kind)
{
	t_ir_base	*ir;

	ir = calloc(1, sizeof(t_ir));
	ir->kind = kind;
	return ((t_ir *)ir);
}

static void	append_stmt(t_ir_stmt_base *ir)
{
	if (g_ir == NULL)
	{
		g_ir = ir;
		g_ir_last = ir;
	}
	else
	{
		g_ir_last->next_stmt = ir;
		g_ir_last = ir;
	}
}

static void	translate_block(t_node *node)
{
	for (;	node != NULL && node->lhs != NULL;
			node = node->rhs)
	{
		translate_stmt(node->lhs);
	}
}

static void	translate_return(t_node *node)
{
	t_ir_return	*ir;

	ir = &malloc_ir(IR_RETURN)->ret;
	append_stmt((t_ir_stmt_base*)ir);
}

static void	translate_stmt(t_node *node)
{
	//t_ir	*ir;

	switch (node->kind)
	{
		case ND_BLOCK:
			translate_block(node);
		case ND_RETURN:
			translate_return(node);
			break ;
		default:
		{
			fprintf(stderr, "translate_stmt : ir isn't implemeanted\nkind : %d\n", node->kind);
			error("Error");
			return ;
		}
	}
}

void	translate_ir(void)
{
	int			i;
	t_ir_func	*func;
	t_deffunc	*def;

	for (i = 0; g_func_defs[i] != NULL; i++)
	{
		def			= g_func_defs[i];
		g_ir		= NULL;
		g_ir_last	= NULL;

		func		= &malloc_ir(IR_FUNC)->func;
		func->def	= def;
		func->name	= my_strndup(def->name, def->name_len);

		translate_stmt(def->stmt);
		func->stmt	= g_ir;
	}
}
