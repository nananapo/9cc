#include <stdlib.h>
#include <stdio.h>

typedef struct s_struct3
{
	int a;
	int b;
	int c;
}	s_struct3;

s_struct3 ret2struct3(void)
{
	s_struct3 *p;
	p = calloc(1, sizeof(s_struct3));
	p->a = 1000;
	p->b = 2000;
	p->c = 3000;
	return (*p);
}

int	main(void)
{
	printf("%d\n", ret2struct3().c);
	return 1;
}
