#include <stdio.h>

int	main(void)
{
	int	i;
	char ptr[100];
	i = 0;
	ptr[0] = 'a';
	ptr[1] = 'b';
	ptr[2] = 'c';

	printf("%c\n", ptr[i]);
	printf("%c\n", ptr[++i]);
	printf("%c\n", ptr[i]);
	printf("%c\n", ptr[i++]);
	printf("%c\n", ptr[i]);
}
