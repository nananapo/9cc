#include <stdio.h>

int	main(void)
{
	int	i;
	int	ptr[100];
	i = 0;
	ptr[0] = 12345;
	ptr[1] = 67890;
	ptr[2] = 29292;

	printf("%d\n", ptr[i]);
	printf("%d\n", ptr[++i]);
	printf("%d\n", ptr[i]);
	printf("%d\n", ptr[i++]);
	printf("%d\n", ptr[i]);
}
