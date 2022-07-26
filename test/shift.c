#include <stdio.h>

int	main(void)
{
	int	i;

	for (i = 0; i < 32; i++)
		printf("%d\n", 1 << i);
	
	for (i = 0; i < 32; i++)
		printf("%d\n", 2147483647 >> i);

	for (i = 0; i < 32; i++)
		printf("%d\n", 1 >> i);
	
	for (i = 0; i < 32; i++)
		printf("%d\n", 2147483647 << i);
}
