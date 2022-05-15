#include <stdio.h>

struct t1
{
	char	a;
	int		b;
};

int	main(void)
{
	printf("%lu\n", sizeof(struct t1));
}
