#include <stdio.h>

int	g_a = 100100;

int	main(void)
{
	printf("%d\n", g_a);
	g_a = 2000;
	printf("%d\n", g_a);
}
