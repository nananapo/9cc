#include <stdio.h>

int	main(void)
{
	int	i;
	char *ptr;
	i = 0;
	ptr = "abcdef";

	printf("%c\n", ptr[i]);
	printf("%c\n", ptr[++i]);
	printf("%c\n", ptr[i]);
	printf("%c\n", ptr[i++]);
	printf("%c\n", ptr[i]);
}
