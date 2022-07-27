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

s_struct1 retstruct1(void);
s_struct1 retstruct1_a(int a);
s_struct2 retstruct2(void);
s_struct2 retstruct2_a(int a);
s_struct3 retstruct3(void);
s_struct3 retstruct3_a(int a);
s_struct4 retstruct4(void);
s_struct4 retstruct4_a(int a);
s_struct5 retstruct5(void);
s_struct5 retstruct5_a(int a);
s_struct6 retstruct6(void);
s_struct6 retstruct6_a(int a);


int	main(void)
{
	printf("1 %d\n", retstruct1().a);
	printf("1 %d\n", retstruct1_a(10000).a);

	printf("2 %d\n", retstruct2().a);
	printf("2 %d\n", retstruct2().b);
	printf("2 %d\n", retstruct2_a(10000).a);
	printf("2 %d\n", retstruct2_a(10000).b);

	printf("3 %d\n", retstruct3().a);
	printf("3 %d\n", retstruct3().b);
	printf("3 %d\n", retstruct3().c);
	printf("3 %d\n", retstruct3_a(10000).a);
	printf("3 %d\n", retstruct3_a(10000).b);
	printf("3 %d\n", retstruct3_a(10000).c);

	printf("4 %d\n", retstruct4().a);
	printf("4 %d\n", retstruct4().b);
	printf("4 %d\n", retstruct4().c);
	printf("4 %d\n", retstruct4().d);
	printf("4 %d\n", retstruct4_a(10000).a);
	printf("4 %d\n", retstruct4_a(10000).b);
	printf("4 %d\n", retstruct4_a(10000).c);
	printf("4 %d\n", retstruct4_a(10000).d);

	printf("5 %d\n", retstruct5().a);
	printf("5 %d\n", retstruct5().b);
	printf("5 %d\n", retstruct5().c);
	printf("5 %d\n", retstruct5().d);
	printf("5 %d\n", retstruct5().e);
	printf("5 %d\n", retstruct5_a(10000).a);
	printf("5 %d\n", retstruct5_a(10000).b);
	printf("5 %d\n", retstruct5_a(10000).c);
	printf("5 %d\n", retstruct5_a(10000).d);
	printf("5 %d\n", retstruct5_a(10000).e);

	printf("6 %d\n", retstruct6().a);
	printf("6 %d\n", retstruct6().b);
	printf("6 %d\n", retstruct6().c);
	printf("6 %d\n", retstruct6().d);
	printf("6 %d\n", retstruct6().e);
	printf("6 %d\n", retstruct6().f);
	printf("6 %d\n", retstruct6_a(10000).a);
	printf("6 %d\n", retstruct6_a(10000).b);
	printf("6 %d\n", retstruct6_a(10000).c);
	printf("6 %d\n", retstruct6_a(10000).d);
	printf("6 %d\n", retstruct6_a(10000).e);
	printf("6 %d\n", retstruct6_a(10000).f);
	return 1;
}
