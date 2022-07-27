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
	debug("# Tokenized\n");

	parseresult = parse(token);
	debug("# Node constructed\n");

	printf(".intel_syntax noprefix\n");
	printf(".p2align	4, 0x90\n");

	// 文字列リテラルを生成する
	lit = parseresult->str_literals;
	while (lit != NULL)
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
		lit = lit->next;
	}

	// コード生成
	i = 0;
	while (parseresult->code[i])
		gen(parseresult->code[i++]);

	return (0);
}
