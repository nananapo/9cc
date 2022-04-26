#include "9cc.h"
#include <stdio.h>

char	*user_input;

Token	*token;

Node	*func_defs[100];
Node	*func_protos[100];
Node	*code[100];

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

//	printf(".section	__TEXT,__text,regular,pure_instructions\n");
	printf(".intel_syntax noprefix\n");
//	printf(".build_version macos, 11, 0	sdk_version 12, 1\n");
	printf(".p2align	4, 0x90\n");

	printf(".global ");
	i = 0;
	while (func_defs[i])
	{
		printf("_%s", func_defs[i]->fname);
		if (func_defs[i + 1])
			printf(", ");
		i++;
	}
	printf("\n");

	i = 0;
	while (code[i])
	{
		gen(code[i++]);
	}

	return (0);
}
