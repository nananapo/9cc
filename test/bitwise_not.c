#include <stdio.h>

int	main(void)
{
	printf("%d\n", ~0);

	int i;
	for (i = 0; i < 31; i = i + 1)
	{
		printf("%d\n", ~(1 << i));
		printf("%d\n", ~~(1 << i));
	}
}
