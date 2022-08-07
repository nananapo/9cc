#include "9cc.h"
#include "list.h"
#include "il.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char			*g_filename;
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
t_il			*g_il;
t_arch			g_arch;

char	*my_strndup(char *source, int len)
{
	char	*dst;

	dst = calloc(len + 1, sizeof(char));
	strncat(dst, source, len);
	return (dst);
}


int main(int argc, char **argv)
{
	g_arch		= ARCH_X8664;
	g_filename	= "-";

	if (argc == 1)
		g_user_input = read_file("-");
	else
	{
		if (strcmp("--arch", argv[1]) == 0)
		{
			if (argc < 3)
			{
				fprintf(stderr, "Usage 9cc --arch [x8664 / riscv] [filename]");
				return (1);
			}

			if (strcmp("x8664", argv[2]) == 0)
				g_arch = ARCH_X8664;
			else if (strcmp("riscv", argv[2]) == 0)
				g_arch = ARCH_RISCV;
			else
			{
				fprintf(stderr, "Usage 9cc --arch [x8664 / riscv] [filename]");
				return (1);
			}

			if (argc < 4)
				g_user_input = read_file("-");
			else
			{
				g_user_input	= read_file(argv[3]);
				g_filename		= argv[3];
			}
		}
		else
		{
			g_user_input	= read_file(argv[1]);
			g_filename		= argv[1];
		}
	}

	g_type_alias = linked_list_new();

	// tokenize
	g_token = tokenize(g_user_input);
	debug("tokenize end");

	// parse
	parse();
	debug("parse end");

	analyze();

	translate_il();

	codegen();
	return (0);
}
