#include "ir.h"
#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern t_ir_func		*g_ir_funcs[1000];
extern t_deffunc		*g_func_defs[1000];

static t_ir_stmt_base	*g_ir;
static t_ir_stmt_base	*g_ir_last;
static int				g_var_id;

static bool	is_stmt_node(t_nodekind kind);
static bool	is_stmt_ir(t_irkind kind);
static t_ir	*malloc_ir(t_irkind kind);
static void	append_stmt(t_ir_stmt_base *ir);
static t_ir_variable	*translate_stmt(t_node *node);
static t_ir_variable	*translate_return(t_node *node);
static t_ir_variable	*translate_num(t_node *node);
static void	translate_block(t_node *node);
static t_ir_variable	*malloc_var(void);

static bool	is_stmt_node(t_nodekind kind)
{
	switch (kind)
	{
		case ND_RETURN:
		case ND_BLOCK:
		case ND_NUM:
			return true;
		default:
			return false;
	}
}

static bool	is_stmt_ir(t_irkind kind)
{
	switch (kind)
	{
		case IR_RETURN:
		case IR_NUMBER:
			return true;
		default:
			return false;
	}
}

static t_ir	*malloc_ir(t_irkind kind)
{
	t_ir_base		*ir;

	ir = calloc(1, sizeof(t_ir));
	ir->kind = kind;

	if (is_stmt_ir(kind))
	{
		((t_ir_stmt_base *)ir)->result 
			= calloc(1, sizeof(t_ir_variable));
	}
	return ((t_ir *)ir);
}

static t_ir_variable	*malloc_var(void)
{
	t_ir_variable	*var;

	var	= calloc(1, sizeof(t_ir_variable));
	var->id = g_var_id++;
	return (var);
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

static t_ir_variable	*translate_num(t_node *node)
{
	t_ir_number		*ir;
	t_ir_variable	*var;

	ir = NULL;
	var = NULL;
	if (node->kind == ND_NUM)
	{
		var = malloc_var();

		ir = &malloc_ir(IR_NUMBER)->number;
		ir->numtype = IRNUM_INT;
		ir->value_int = node->val;
		ir->result = var;
	}
	else
	{
		error("no");
		return (NULL);
	}

	append_stmt((t_ir_stmt_base*)ir);
	return (var);
}

static t_ir_variable	*translate_return(t_node *node)
{
	t_ir_return		*ir;
	t_ir_variable	*var;

	var = malloc_var();

	ir = &malloc_ir(IR_RETURN)->ret;
	ir->has_value = node->lhs != NULL;
	ir->result = var;

	if (ir->has_value)
	{
		ir->result = translate_stmt(node->lhs);
	}
	append_stmt((t_ir_stmt_base*)ir);
	return (var);
}

static t_ir_variable	*translate_stmt(t_node *node)
{
	t_ir_variable	*var;

	if (!is_stmt_node(node->kind))
	{
		fprintf(stderr, "translate_stmt : node is not stmt\nkind : %d\n", node->kind);
		error("Error");
		return (NULL);
	}

	switch (node->kind)
	{
		case ND_BLOCK:
		{
			translate_block(node);
			return (NULL);
		}
		case ND_RETURN:
		{
			translate_return(node);
			return (NULL);
		}
		case ND_NUM:
		{
			var = translate_num(node);
			return (var);
		}
		default:
		{
			fprintf(stderr, "translate_stmt : ir isn't implemeanted\nkind : %d\n", node->kind);
			error("Error");
			return (NULL);
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
