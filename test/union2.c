#include <stdio.h>

typedef union u_a
{
	struct s_a
	{
		int a;
		int b;
		int c;
	} a;
	int b[3];
}	my_union;

int	main(void)
{
	my_union a;

	a.a.a = 12345;
	a.a.b = 387654;
	a.a.c = 876234;

	printf("%d\n", a.b[0]);
	printf("%d\n", a.b[1]);
	printf("%d\n", a.b[2]);
}
