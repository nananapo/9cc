#include <stdio.h>

int	main(void)
{
	int i = 0;
	char *program = "[aaa[]aa[]a[aa[a[a]a]a]a]";
	while (*(++program) != '\0' && !(i == 0 && *program == ']'))
	{
		printf("%c\n", *program);
		if (*program == '[') i++;
		else if (*program == ']') i--;
	}
	printf("%s\n", program);
}
