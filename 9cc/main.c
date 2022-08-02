#include "9cc.h"
#include "list.h"
#include <stdio.h>
#include <string.h>

# define N_MAX 100

char			*g_user_input;

t_token			*g_token;
t_deffunc		*g_func_defs[1000];
t_deffunc		*g_func_protos[1000];
t_defvar		*g_global_vars[1000];
t_str_elem		*g_str_literals[1000];
t_defstruct		*g_struct_defs[1000];
t_defenum		*g_enum_defs[1000];
t_defunion		*g_union_defs[1000];
t_deffunc		*g_func_now;
t_linked_list	*g_type_alias;

int main(int argc, char **argv)
{
	if (argc == 1)
		g_user_input = read_file("-");
	else if (argc == 2)
		g_user_input = read_file(argv[1]);
	else
	{
		fprintf(stderr, "引数の個数が正しくありません");
		return (1);
	}

	g_type_alias = linked_list_new();

	// tokenize
	g_token = tokenize(g_user_input);
	debug("tokenize end");

	// parse
	parse();
	debug("parse end");

	// analyze
	analyze();

	// codegen
	codegen();

	return (0);
}
