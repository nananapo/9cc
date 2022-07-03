int dint(int a);

struct t1
{
	int *a;
	int b;
};

int test(int b, int c, int d, int e, struct t1 a, int f)
{
	dint(*a.a);
	dint(a.b);

	*a.a = 42;
	a.b = 33;

	dint(*a.a);
	dint(a.b);

	dint(b);
	dint(c);
	dint(d);
	dint(e);
	dint(f);
	return 1;
}

int main()
{
	struct t1 v1;
	int	a;

	a = 21;

	v1.a = &a;
	v1.b = 22;

	dint(*v1.a);
	dint(v1.b);

	test(515, 323, 142, 285, v1, 909);

	dint(*v1.a);
	dint(v1.b);
}
