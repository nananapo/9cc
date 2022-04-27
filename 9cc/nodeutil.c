#include "9cc.h"
#include <stdlib.h>
#include <string.h>

extern Node		*func_defs[];
extern Node		*func_protos[];

Node	*get_function_by_name(char *name, int len)
{
	int i = 0;
	while (func_defs[i])
	{
	Node *tmp = func_defs[i];
		if (tmp->flen == len && strncmp(tmp->fname, name, len) == 0)
			return tmp;
		i++;
	}

	i = 0;
	while (func_protos[i])
	{
		Node *tmp = func_protos[i];
		if (tmp->flen == len && strncmp(tmp->fname, name, len) == 0)
			return tmp;
		i++;
	}
	return NULL;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;

	// for
	node->for_init = NULL;
	node->for_if = NULL;
	node->for_next = NULL;

	// else
	node->els = NULL;
	
	// call & deffunc
	node->fname = NULL;
	node->args = NULL;

	node->type = NULL;
	return node;
}

Node *new_node_num(int val)
{
	Node *node = new_node(ND_NUM, NULL, NULL);
	node->val = val;
	node->type = new_primitive_type(INT);
	return node;
}

