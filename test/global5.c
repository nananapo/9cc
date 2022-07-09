#include <stdio.h>

char	g_a[4] = {'a', 'z', 'A', 'Z'};

int	main(void)
{
	printf("%c\n", g_a[0]);
	printf("%c\n", g_a[1]);
	printf("%c\n", g_a[2]);
	printf("%c\n", g_a[3]);
}
