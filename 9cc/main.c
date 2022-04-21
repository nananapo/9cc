#include "9cc.h"
#include <stdio.h>

char	*user_input;

Token	*token;

Node	*code[100];

LVar	*locals = NULL;

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
	printf(".global _main\n");
	printf("_main:\n");
	
	printf("    push rbp\n");
	printf("    mov rbp, rsp\n");
	printf("    sub rsp, 208\n");

	i = 0;
	while (code[i])
	{
		gen(code[i++]);
		printf("    pop rax\n");
	}

	printf("    mov rsp, rbp\n");
	printf("    pop rbp\n");
	printf("    ret\n");
	return (0);
}
