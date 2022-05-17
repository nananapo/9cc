struct t1
{
	int a;
	int b;
	int c;
	int d;
	int e;
};

struct t2
{
	int a;
};

struct t3
{
	char a;
};

int	test(int a, int b, int c, int d, int e, int f
		,int g, int h, int i)
{
	i = 199;
	return a + b + c + d + e + f + g + h + i;
}

int test1(struct t1 a, int d)
{
	a.d = 100;
	return a.c;
}

int test2(struct t2 a)
{
	return 0;
}

int test3(struct t3 a)
{
	return 0;
}

int	main(void)
{
	struct t1 s;
	test(10, 11, 12, 13, 14, 15, 16, 17, 18);

	test1(s, 123);
}
