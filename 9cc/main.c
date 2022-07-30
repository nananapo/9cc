#include "9cc.h"
#include <stdio.h>
#include <string.h>

# define N_MAX 100

char			*g_user_input;

Token			*g_token;
t_deffunc		*g_func_defs[1000];
t_deffunc		*g_func_protos[1000];
t_defvar		*g_global_vars[1000];
t_str_elem		*g_str_literals;
StructDef		*g_struct_defs[1000];
EnumDef			*g_enum_defs[1000];
UnionDef		*g_union_defs[1000];
LVar			*g_locals;
t_deffunc		*g_func_now;
t_linked_list	*g_type_alias;

int main(int argc, char **argv)
{
	int			i;
	t_str_elem	*lit;
	Token		*token;

	if (argc == 1)
		g_user_input = read_file("-");
	else if (argc == 2)
		g_user_input = read_file(argv[1]);
	else
	{
		fprintf(stderr, "引数の個数が正しくありません");
		return (1);
	}

	token = tokenize(g_user_input);
	debug("Tokenized");

	parse(token);
	debug("Node constructed");

	printf(".intel_syntax noprefix\n");
	printf(".p2align	4, 0x90\n");

	// 文字列リテラル生成
	for (lit = g_str_literals; lit != NULL; lit = lit->next)
	{
		printf("%s:\n", get_str_literal_name(lit->index));
		printf("    .string \"");
		i = -1;
		while (++i < lit->len)
		{
			if (lit->str[i] == '\\')
			{
				i++;
				if (lit->str[i] != '\'')
					printf("\\");
			}
			printf("%c", lit->str[i]);
		}
		printf("\"\n");
	}

	// グローバル変数を生成
	printf(".section	__DATA, __data\n");
	for (i = 0; g_global_vars[i] != NULL; i++)
		gen_defglobal(g_global_vars[i]);

	// 関数を生成
	printf(".section	__TEXT,__text,regular,pure_instructions\n");
	for (i = 0; g_func_defs[i] != NULL; i++)
		gen_deffunc(g_func_defs[i]);

	return (0);
}
