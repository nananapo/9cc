#include "9cc.h"
#include <stdio.h>
#include <string.h>

# define N_MAX 100

char		*user_input;

Token		*token;

Node		*func_defs[N_MAX];
Node		*func_protos[N_MAX];
Node		*code[N_MAX];

Node		*global_vars[N_MAX];
t_str_elem	*str_literals;

StructDef	*struct_defs[N_MAX];

int main(int argc, char **argv)
{
	char		*str;
	int			sign;
	int			i;
	t_str_elem	*lit;

	if (argc != 2)
	{
		fprintf(stderr, "引数の個数が正しくありません");
		return (1);
	}

	user_input = argv[1];
	token = tokenize(argv[1]);
	printf("# Tokenized\n");

	program();
	printf("# Node constructed\n");

	printf(".intel_syntax noprefix\n");
	printf(".p2align	4, 0x90\n");

	// 文字列リテラルを生成する
	lit = str_literals;
	while (lit != NULL)
	{
		printf("%s:\n", get_str_literal_name(lit->index));
		printf("    .string \"%s\"\n", strndup(lit->str, lit->len));
		lit = lit->next;
	}

	// コード生成
	i = 0;
	while (code[i])
		gen(code[i++]);

	return (0);
}
