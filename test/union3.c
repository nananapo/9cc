#include <stdio.h>

union	u_a
{
	int	a;
	char b[4];
};

int	main(void)
{
	union u_a	c;
	union u_a	*d;

	d = &c;
	d->a = 12345678;
	printf("a      : %d\n", d->a);
	printf("b[0]   : %d\n", (int)d->b[0]);
	printf("b[1]   : %d\n", (int)d->b[1]);
	printf("b[2]   : %d\n", (int)d->b[2]);
	printf("b[3]   : %d\n", (int)d->b[3]);
}
