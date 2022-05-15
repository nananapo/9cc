#include <stdio.h>

struct test
{
	char	a;
	char	b;
	char	c;
};

int main(void)
{
	printf("%lu\n", sizeof(struct test));
}
