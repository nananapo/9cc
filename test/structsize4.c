int	pint(int a);
int pline();

struct t1
{
	char a;
};

struct t2
{
	struct t1 a;
};

struct t3
{
	struct t2 a;
};

struct t4
{
	struct t1 a;
	struct t2 b;
	struct t3 c;
};

struct t5
{
	struct t1	a;
	int 		b;
};

int main()
{
	struct t1 a;
	struct t2 b;
	struct t3 c;
	struct t4 d;
	struct t5 e;

	pint(sizeof(a)); pline();
	pint(sizeof(b)); pline();
	pint(sizeof(c)); pline();
	pint(sizeof(d)); pline();
	pint(sizeof(e)); pline();
}
