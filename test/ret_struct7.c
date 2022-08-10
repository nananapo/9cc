#include <stdlib.h>
#include <stdio.h>

typedef struct s_struct5
{
	int a;
	int b;
	int c;
	int d;
	int e;
} s_struct5;

s_struct5 ret2struct5(void)
{
	s_struct5 *p;
	p = calloc(1, sizeof(s_struct5));
	p->a = 1000;
	p->b = 2000;
	p->c = 3000;
	p->d = 4000;
	p->e = 5000;
	return (*p);
}

int	main(void)
{
	printf("5 %d\n", ret2struct5().a);
	return 1;
}
