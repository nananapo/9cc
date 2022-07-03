int dint(int a);

struct t1
{
	int *a;
	int b;
};

int test(int b, int c, int d, int e, struct t1 a)
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

	test(3,4,5,6,v1);

	dint(*v1.a);
	dint(v1.b);
}
