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

static int	my_strlen(char *c)
{
	int i;
	i = 0;
	while (c[i] != '\0')
		i++;
	return (i);
}

int my_print(char *c)
{
	write(1, c, my_strlen(c));
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
//	write(2, "called malloc\n", 14);
	int* ptr = (int *)malloc(sizeof(int) * size);
//	printf("ptr %p\n", ptr);
	return ptr;
}

int	pcheck()
{
	write(1, "check\n", 6);
	return 0;
}


typedef struct s_struct1
{
	int a;
}	s_struct1;


typedef struct s_struct2
{
	int a;
	int b;
}	s_struct2;

typedef struct s_struct3
{
	int a;
	int b;
	int c;
}	s_struct3;

typedef struct s_struct4
{
	int a;
	int b;
	int c;
	int d;
} s_struct4;

typedef struct s_struct5
{
	int a;
	int b;
	int c;
	int d;
	int e;
} s_struct5;

typedef struct s_struct6
{
	int a;
	int b;
	int c;
	int d;
	int e;
	int f;
} s_struct6;

s_struct1 retstruct1(void)
{
	s_struct1 *p;
	p = calloc(1, sizeof(s_struct1));
	p->a = 1000;
	return (*p);
}

s_struct1 retstruct1_a(int a)
{
	s_struct1 *p;
	p = calloc(1, sizeof(s_struct1));
	p->a = a + 100;
	return (*p);
}


s_struct2 retstruct2(void)
{
	s_struct2 *p;
	p = calloc(1, sizeof(s_struct2));
	p->a = 1000;
	p->b = 2000;
	return (*p);
}

s_struct2 retstruct2_a(int a)
{
	s_struct2 *p;
	p = calloc(1, sizeof(s_struct2));
	p->a = a + 100;
	p->b = a + 200;
	return (*p);
}



s_struct3 retstruct3(void)
{
	s_struct3 *p;
	p = calloc(1, sizeof(s_struct3));
	p->a = 1000;
	p->b = 2000;
	p->c = 3000;
	return (*p);
}

s_struct3 retstruct3_a(int a)
{
	s_struct3 *p;
	p = calloc(1, sizeof(s_struct3));
	p->a = a + 100;
	p->b = a + 200;
	p->c = a + 300;
	return (*p);
}


s_struct4 retstruct4(void)
{
	s_struct4 *p;
	p = calloc(1, sizeof(s_struct4));
	p->a = 1000;
	p->b = 2000;
	p->c = 3000;
	p->d = 4000;
	return (*p);
}

s_struct4 retstruct4_a(int a)
{
	s_struct4 *p;
	p = calloc(1, sizeof(s_struct4));
	p->a = a + 100;
	p->b = a + 200;
	p->c = a + 300;
	p->d = a + 400;
	return (*p);
}



s_struct5 retstruct5(void)
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

s_struct5 retstruct5_a(int a)
{
	s_struct5 *p;
	p = calloc(1, sizeof(s_struct5));
	p->a = a + 100;
	p->b = a + 200;
	p->c = a + 300;
	p->d = a + 400;
	p->e = a + 500;
	return (*p);
}



s_struct6 retstruct6(void)
{
	s_struct6 *p;
	p = calloc(1, sizeof(s_struct6));
	p->a = 1000;
	p->b = 2000;
	p->c = 3000;
	p->d = 4000;
	p->e = 5000;
	p->f = 6000;
	return (*p);
}

s_struct6 retstruct6_a(int a)
{
	s_struct6 *p;
	p = calloc(1, sizeof(s_struct6));
	p->a = a + 100;
	p->b = a + 200;
	p->c = a + 300;
	p->d = a + 400;
	p->e = a + 500;
	p->f = a + 600;
	return (*p);
}


#include <math.h>
float mysin(float a)
{
	return (float)sin(a);
}

float mycos(float b)
{
	return (float)cos(b);
}

void putf(float a)
{
	printf("%f\n", a);
}
