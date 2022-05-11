#include "9cc.h"
#include <stddef.h>

extern Node	*code[];
extern Node	*func_defs[];

static void	type_check(Node *node)
{
	switch(node->kind)
	{
		case ND_FUNCDEF:
			type_check(node->lhs);
			
			break;
		case ND_PROTOTYPE:
			break;
		default:
			break;
	}
}

void	start_type_path(void)
{
	int	i;

	i = -1;
	while (code[++i] != NULL)
		type_check(code[i]);
}
