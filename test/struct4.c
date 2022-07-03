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

	pint(sizeof(a)); pline();
	pint(sizeof(b)); pline();

	a.b = &b;
	a.b->a = 200;

	pint(a.b->a); pline();
	pint(b.a); pline();
}
