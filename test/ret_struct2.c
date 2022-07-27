#include <stdlib.h>
#include <stdio.h>

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


s_struct1 ret2struct1(void)
{
	s_struct1 *p;
	p = calloc(1, sizeof(s_struct1));
	p->a = 1000;
	return (*p);
}

s_struct1 ret2struct1_a(int a)
{
	s_struct1 *p;
	p = calloc(1, sizeof(s_struct1));
	p->a = a + 100;
	return (*p);
}


s_struct2 ret2struct2(void)
{
	s_struct2 *p;
	p = calloc(1, sizeof(s_struct2));
	p->a = 1000;
	p->b = 2000;
	return (*p);
}

s_struct2 ret2struct2_a(int a)
{
	s_struct2 *p;
	p = calloc(1, sizeof(s_struct2));
	p->a = a + 100;
	p->b = a + 200;
	return (*p);
}



s_struct3 ret2struct3(void)
{
	s_struct3 *p;
	p = calloc(1, sizeof(s_struct3));
	p->a = 1000;
	p->b = 2000;
	p->c = 3000;
	return (*p);
}

s_struct3 ret2struct3_a(int a)
{
	s_struct3 *p;
	p = calloc(1, sizeof(s_struct3));
	p->a = a + 100;
	p->b = a + 200;
	p->c = a + 300;
	return (*p);
}


s_struct4 ret2struct4(void)
{
	s_struct4 *p;
	p = calloc(1, sizeof(s_struct4));
	p->a = 1000;
	p->b = 2000;
	p->c = 3000;
	p->d = 4000;
	return (*p);
}

s_struct4 ret2struct4_a(int a)
{
	s_struct4 *p;
	p = calloc(1, sizeof(s_struct4));
	p->a = a + 100;
	p->b = a + 200;
	p->c = a + 300;
	p->d = a + 400;
	return (*p);
}



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

s_struct5 ret2struct5_a(int a)
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



s_struct6 ret2struct6(void)
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

s_struct6 ret2struct6_a(int a)
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

int	main(void)
{
	printf("1 %d\n", ret2struct1().a);
	printf("1 %d\n", ret2struct1_a(10000).a);

	printf("2 %d\n", ret2struct2().a);
	printf("2 %d\n", ret2struct2().b);
	printf("2 %d\n", ret2struct2_a(10000).a);
	printf("2 %d\n", ret2struct2_a(10000).b);

	printf("3 %d\n", ret2struct3().a);
	printf("3 %d\n", ret2struct3().b);
	printf("3 %d\n", ret2struct3().c);
	printf("3 %d\n", ret2struct3_a(10000).a);
	printf("3 %d\n", ret2struct3_a(10000).b);
	printf("3 %d\n", ret2struct3_a(10000).c);

	printf("4 %d\n", ret2struct4().a);
	printf("4 %d\n", ret2struct4().b);
	printf("4 %d\n", ret2struct4().c);
	printf("4 %d\n", ret2struct4().d);
	printf("4 %d\n", ret2struct4_a(10000).a);
	printf("4 %d\n", ret2struct4_a(10000).b);
	printf("4 %d\n", ret2struct4_a(10000).c);
	printf("4 %d\n", ret2struct4_a(10000).d);

	printf("5 %d\n", ret2struct5().a);
	printf("5 %d\n", ret2struct5().b);
	printf("5 %d\n", ret2struct5().c);
	printf("5 %d\n", ret2struct5().d);
	printf("5 %d\n", ret2struct5().e);
	printf("5 %d\n", ret2struct5_a(10000).a);
	printf("5 %d\n", ret2struct5_a(10000).b);
	printf("5 %d\n", ret2struct5_a(10000).c);
	printf("5 %d\n", ret2struct5_a(10000).d);
	printf("5 %d\n", ret2struct5_a(10000).e);

	printf("6 %d\n", ret2struct6().a);
	printf("6 %d\n", ret2struct6().b);
	printf("6 %d\n", ret2struct6().c);
	printf("6 %d\n", ret2struct6().d);
	printf("6 %d\n", ret2struct6().e);
	printf("6 %d\n", ret2struct6().f);
	printf("6 %d\n", ret2struct6_a(10000).a);
	printf("6 %d\n", ret2struct6_a(10000).b);
	printf("6 %d\n", ret2struct6_a(10000).c);
	printf("6 %d\n", ret2struct6_a(10000).d);
	printf("6 %d\n", ret2struct6_a(10000).e);
	printf("6 %d\n", ret2struct6_a(10000).f);
	return 1;
}
