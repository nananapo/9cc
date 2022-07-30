#include "9cc.h"
#include <stdio.h>
#include <string.h>

# define N_MAX 100

char		*user_input;

int main(int argc, char **argv)
{
	int			i;
	t_str_elem	*lit;
	Token		*token;
	ParseResult	*parseresult;

	if (argc == 1)
		user_input = read_file("-");
	else if (argc == 2)
		user_input = read_file(argv[1]);
	else
	{
		fprintf(stderr, "引数の個数が正しくありません");
		return (1);
	}

	token = tokenize(user_input);
	debug("Tokenized");

	parseresult = parse(token);
	debug("Node constructed");

	printf(".intel_syntax noprefix\n");
	printf(".p2align	4, 0x90\n");

	// 文字列リテラルを生成
	for (lit = parseresult->str_literals; lit != NULL; lit = lit->next)
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
	for (i = 0; parseresult->global_vars[i] != NULL; i++)
		gen_defglobal(parseresult->global_vars[i]);

	// 関数を生成
	for (i = 0; parseresult->func_defs[i] != NULL; i++)
		gen_deffunc(parseresult->func_defs[i]);

	return (0);
}
