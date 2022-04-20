#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	char	*str;
	int		sign;

	if (argc != 2)
	{
		fprintf(stderr, "引数の個数が正しくありません");
		return (1);
	}
	str = argv[1];
	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");
	printf("    mov rax, %ld\n", strtol(str, &str, 10));
	while (*str)
	{
		if (*str ==  '-')
			sign = -1;
		else if (*str == '+')
			sign = 1;
		else
		{
			fprintf(stderr, "予期しない文字です。 '%c'\n", *str);
			return 1;
		}
		str++;

		if (sign == 1)
			printf("    add rax, %ld\n", strtol(str, &str, 10));
		else
			printf("    sub rax, %ld\n", strtol(str, &str, 10));
	}
	printf("    ret\n");
	return (0);
}
