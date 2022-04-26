#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static void	my_putchar(char c)
{
	write(1, &c, 1);
}

static int	recpint(int a)
{
	if (-9 <= a && a <= 9)
	{
		if (a < 0)
			a = -a;
		my_putchar('0' + a);
		return 0;
	}
	recpint(a / 10);
	recpint(a % 10);
	return 0;
}

int	pint(int a)
{
	if (a < 0)
		my_putchar('-');
	recpint(a);
	return 0;
}

int	pspace(int count)
{
	int	i;

	i = 0;
	while (i++ < count)
		my_putchar(' ');
	return 0;
}

int	pline()
{
	my_putchar('\n');
	return 0;
}

int pptr(int *ptr)
{
	printf("ptr : %p\n", ptr);
	return 0;
}

int *my_malloc_int(int size)
{
	write(2, "called malloc\n", 14);
	int* ptr = (int *)malloc(sizeof(int) * size);
//	printf("ptr %p\n", ptr);
	return ptr;
}

int	pcheck()
{
	write(1, "check\n", 6);
	return 0;
}
