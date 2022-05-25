int dint(int a);

struct t1
{
	int *a;
	int b;
};

int test(int d, struct t1 a, int e, struct t1 b, int f, struct t1 c, int g)
{
	dint(*c.a);
	dint(c.b);
	return 1;
}

int main()
{
	struct t1 v1;
	struct t1 v2;
	struct t1 v3;

	int a;
	a = 42;

	v3.a = &a;
	v3.b = 34567;

	test(192, v1, 223, v2, 653, v3, 987);
}
