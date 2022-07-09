#include <stdio.h>

typedef enum e_test
{
	Test
} testenum;

typedef struct s_test
{
	testenum	e;
	void		*v;
} teststruct;

int main(void)
{
	teststruct a;

	printf("%d\n", sizeof(a));
	printf("%d\n", (void *)&a.e - (void *)&a);
	printf("%d\n", (void *)&a.v - (void *)&a);
}
