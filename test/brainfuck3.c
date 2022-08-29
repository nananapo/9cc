#include <stdio.h>

int test(int a)
{
	printf("t%d\n", a);
	return (1);
}

int	main(void)
{
	int a = 10;
	while (--a != 0 && test(a))
	{
		printf("%d\n", a);
	}
}
