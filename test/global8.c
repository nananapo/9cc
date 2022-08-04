#include <stdio.h>
#include <stdlib.h>

typedef struct s_test
{
	int	a;
	int	b;
}	t_test;

t_test	*ptr;

void	change(t_test **p)
{
//	*p = malloc(sizeof(t_test));
//	(*p)->a = 1000;
//	(*p)->b = 2000;
}

int	main(void)
{
	printf("is NULL %d\n", ptr == 0);
	change(&ptr);
	printf("is NULL %d\n", ptr == 0);
	printf("ptr->a %d\n", ptr->a);
	printf("ptr->b %d\n", ptr->b);
}
