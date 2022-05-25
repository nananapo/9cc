int dint(int a);

struct t1
{
	int *a;
	int b;
};

int test(int d, struct t1 a, int e, struct t1 b, int f, struct t1 c, int g)
{
	dint(*a.a);
	dint(a.b);

	dint(*b.a);
	dint(b.b);

	dint(*c.a);
	dint(c.b);


	dint(d);
	dint(e);
	dint(f);
	dint(g);

	*a.a = 42;
	a.b = 33;

	*b.a = 18;
	b.b = 222;

	*c.a = 54;
	c.b = 21;

	dint(*a.a);
	dint(a.b);

	dint(*b.a);
	dint(b.b);

	dint(*c.a);
	dint(c.b);

	dint(d);
	dint(e);
	dint(f);
	dint(g);

	return 1;
}

int main()
{
	struct t1 v1;
	struct t1 v2;
	struct t1 v3;

	int	a;
	int	b;
	int	c;

	a = 21;
	b = 99;
	c = 8282;

	v1.a = &a;
	v1.b = 22;

	v2.a = &b;
	v2.b = 999;

	v3.a = &c;
	v3.b = 777;

	dint(*v1.a);
	dint(v1.b);

	dint(*v2.a);
	dint(v2.b);

	dint(*v3.a);
	dint(v3.b);

	test(192, v1, 223, v2, 653, v3, 987);

	dint(*v1.a);
	dint(v1.b);

	dint(*v2.a);
	dint(v2.b);

	dint(*v3.a);
	dint(v3.b);
}
