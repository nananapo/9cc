int pint(int x);
int pline();

struct t1
{
	int	a;
};

struct t2
{
	struct t1	*a;
	struct t1	b;
};

int	main()
{
	struct t1 v1;
	struct t1 v2;
	struct t1 *v3;

	struct t2 v4;

	v1.a = 210;
	pint(v1.a); pline();
	v2.a = 818181;
	pint(v2.a); pline();
	v1 = v2;
	pint(v1.a == v2.a); pline();
	pint(v1.a); pline();

	v1.a = 333;
	v3 = &v1;
	pint(v3->a); pline();
	v1.a = 789;
	pint(v3->a); pline();

	v4.b = *v3;
	v4.a = &v2;

	pint(v4.a->a); pline();
	pint(v4.b.a); pline();

	v4.a->a = 999;
	pint(v2.a); pline();
	v3->a = 12345;
	pint(v3->a); pline();
	pint(v4.b.a); pline();
}
