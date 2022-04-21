#include "9cc.h"
#include <stdio.h>

Token		*token;
char	*user_input;

int main(int argc, char **argv)
{
	char	*str;
	int		sign;
	Node	*node;

	if (argc != 2)
	{
		fprintf(stderr, "引数の個数が正しくありません");
		return (1);
	}

	user_input = argv[1];
	token = tokenize(argv[1]);
	node = expr();

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");
	
	gen(node);

	printf("    pop rax\n");
	printf("    ret\n");
	return (0);
}
