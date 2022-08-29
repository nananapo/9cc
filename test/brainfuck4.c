#include <stdio.h>

int test(char a)
{
	printf("%c\n", a);
	return (1);
}

int	main(void)
{
	int i = 0;
	char *program = "HelloWorld";
	while (*(++program) != '\0' && *program != ']' && test(*program))
	{
		test(*program);
	}
}
