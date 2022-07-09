#include <stdio.h>

char	*g_a[4] = {"JKL", "GHI", "DEF", "ABC"};

int	main(void)
{
	printf("%s\n", g_a[0]);
	printf("%s\n", g_a[1]);
	printf("%s\n", g_a[2]);
	printf("%s\n", g_a[3]);
}
