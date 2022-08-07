int dint(int a);

struct t1
{
	int *a;
	int b;
};

int test(int a, int b, int c, int d, int e, int f, struct t1 g)
{
	dint(a);
	dint(b);
	dint(c);
	dint(d);
	dint(e);
	dint(f);

	dint(*g.a);
	dint(g.b);

	return 1;
}

int main()
{
	int			a;
	struct t1	v1;

	a = 1000;

	v1.a = &a;
	v1.b = 123;

	test(1, 2, 3, 4, 5, 6, v1);
}
