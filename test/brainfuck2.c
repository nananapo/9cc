#include <stdio.h>

int test1(void)
{
	printf("test1\n");
	return 1;
}
int test2(void)
{
	printf("test2\n");
	return 1;
}
int test3(void)
{
	printf("test3\n");
	return 1;
}

int	main(void)
{
	if (test1() && test2() && test3())
		return 0;
}
