#include <stdio.h>

struct t1
{
	int a;
	char b;
};

struct t2
{
	char a[5];
};


struct t3
{
	char a;
};

struct t4
{
	struct t3 a[5];
};

struct t5
{
	struct t3 a;
	struct t3 b;
	struct t3 c;
	struct t3 d;
	struct t3 e;
};

struct t6
{
	struct t5 a;
};

struct t7
{
	struct t5 a;
	struct t5 b;
};

int	main(void)
{
	struct t1 s;
	struct t2 s2;
	struct t4 s3;
	struct t5 s4;
	struct t6 s5;
	struct t7 s6;

	printf("t1		: %p\n", &s);
	printf("size	: %lu\n", sizeof(s));
	printf("a		: %p\n", &s.a);
	printf("b		: %p\n", &s.b);

	printf("t2		: %p\n", &s2);
	printf("size	: %lu\n", sizeof(s2));
	printf("a		: %p\n", &s2.a);

	printf("t4		: %p\n", &s3);
	printf("size	: %lu\n", sizeof(s3));
	printf("a		: %p\n", &s3.a);

	printf("t5		: %p\n", &s4);
	printf("size	: %lu\n", sizeof(s4));
	printf("a		: %p\n", &s4.a);

	printf("t6		: %p\n", &s5);
	printf("size	: %lu\n", sizeof(s5));

	printf("t7		: %p\n", &s6);
	printf("size	: %lu\n", sizeof(s6));
	printf("a		: %p\n", &s6.a);
	printf("b		: %p\n", &s6.b);

}
