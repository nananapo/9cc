#include <stdio.h>

union	u_a
{
	int	a;
	char b[4];
};

int	main(void)
{
	union u_a	c;

	c.a = 12345678;
	printf("sizeof : %lu\n", sizeof(union u_a));
	printf("a      : %d\n", c.a);
	printf("b[0]   : %d\n", (int)c.b[0]);
	printf("b[1]   : %d\n", (int)c.b[1]);
	printf("b[2]   : %d\n", (int)c.b[2]);
	printf("b[3]   : %d\n", (int)c.b[3]);
}
