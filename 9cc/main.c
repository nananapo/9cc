#include "9cc.h"
#include <stdio.h>

char	*user_input;

Token	*token;

Node	*func_defs[100];
Node	*func_protos[100];
Node	*code[100];

Node	*global_vars[100];

int main(int argc, char **argv)
{
	char	*str;
	int		sign;
	int		i;

	if (argc != 2)
	{
		fprintf(stderr, "引数の個数が正しくありません");
		return (1);
	}

	user_input = argv[1];
	token = tokenize(argv[1]);
	program();

	printf(".intel_syntax noprefix\n");
	printf(".p2align	4, 0x90\n");

	printf(".globl ");
	i = 0;
	while (func_defs[i])
	{
		printf("_%s", func_defs[i]->fname);
		if (func_defs[i + 1])
			printf(", ");
		i++;
	}
	i = 0;
	while (global_vars[i])
	{
		if (func_defs[0] != NULL)
			printf(",");
		printf("_%s", global_vars[i]->var_name);
		i++;
	}
	printf("\n");

	i = 0;
	while (code[i])
		gen(code[i++]);

	return (0);
}
