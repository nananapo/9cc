int dint(int a);

struct t1
{
	int *a;
	int b;
	int c;
	int *d;
	int *e;
	int *f;
};

int test(struct t1 a, struct t1 b, struct t1 c, int i)
{
	dint(*a.a);
	dint(a.b);

	dint(*b.a);
	dint(b.b);

	dint(*c.a);
	dint(c.b);

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

	dint(i);

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

	test(v1, v2, v3, 999);

	dint(*v1.a);
	dint(v1.b);

	dint(*v2.a);
	dint(v2.b);

	dint(*v3.a);
	dint(v3.b);
}
