int pint(int n);
int pline();

struct t2
{
	int a;
	char b;
};

struct t1
{
	struct t2 a[3];
	struct t2 *b;
};

int main()
{
	struct t1 a;
	struct t2 b;

	a.b = &b;
	a.b->a = 200;

	a.a[1].a = 100;
	a.a[2].a = 65535;
	a.a[0].a = 2147483647;

	pint(a.b->a); pline();
	pint(b.a); pline();

	pint(a.a[0].a); pline();
	pint(a.a[1].a); pline();
	pint(a.a[2].a); pline();
}
