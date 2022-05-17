#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int	my_putchar(char c)
{
	write(1, &c, 1);
	return 0;
}

int my_putstr(char *c, int n)
{
	write(1, c, n);
	return 0;
}

int my_print(char *c)
{
	printf("%s", c);
	return 0;
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

int pchar(char n)
{
	my_putchar(n);
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
	printf("ptr		: %p\n", ptr);
	printf("value	: %d\n", *ptr);
	return 0;
}

int pptrc(char *ptr)
{
	printf("ptrc	: %p\n", ptr);
	printf("value	: %d\n", *ptr);
	return 0;
}

int	ptr42(int *ptr)
{
	*ptr = 42;
	pint(*ptr);
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
