#include <stdio.h>

int	pint(int a)
{
	printf("%d",a);
	return 0;
}

int	pspace(int count)
{
	int	i;

	i = 0;
	while (i++ < count)
		printf(" ");
	return 0;
}

int	pline()
{
	printf("\n");
	return 0;
}
