#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define LL long long

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

static void	recpll(LL a)
{
	if (-9 <= a && a <= 9)
	{
		if (a < 0)
			a = -a;
		my_putchar('0' + a);
		return;
	}
	recpll(a / 10);
	recpll(a % 10);
	return;
}

void	pll(LL a)
{
	if (a < 0)
		my_putchar('-');
	recpll(a);
	return;
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

int dint(int a)
{
	if (a < 0)
		my_putchar('-');
	recpint(a);
	my_putchar('\n');
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

void	pptrv(void *ptr)
{
	pll((LL)ptr);
}

int	ptr42(int *ptr)
{
	*ptr = 42;
	pint(*ptr);
	return 0;
}

void put_charptrv(void *ptr)
{
	printf("%c\n", *(char *)ptr);
}

void put_strptrv(void *ptr)
{
	printf("%s\n", *(char **)ptr);
}

void put_intptrv(void *ptr)
{
	printf("%d\n", *(int *)ptr);
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
