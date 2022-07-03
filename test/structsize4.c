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

struct t6
{
	struct t1 a[5];
};

struct t7
{
	struct t6 a[2];
};

struct t8
{
	char a[10];
	int  b[3];
};

struct t9
{
	char a[10][5];
};

struct t10
{
	struct t9 a[5][11];
};

struct t11
{
	struct t9 a[5][11];
	struct t8 b[3];
};

int main()
{
	struct t1 a;
	struct t2 b;
	struct t3 c;
	struct t4 d;
	struct t5 e;
	struct t6 f;
	struct t7 g;
	struct t8 h;
	struct t9 i;
	struct t10 j;
	struct t10 k;

	pint(sizeof(a)); pline();
	pint(sizeof(b)); pline();
	pint(sizeof(c)); pline();
	pint(sizeof(d)); pline();
	pint(sizeof(e)); pline();
	pint(sizeof(f)); pline();
	pint(sizeof(g)); pline();
	pint(sizeof(h)); pline();
	pint(sizeof(i)); pline();
	pint(sizeof(j)); pline();
	pint(sizeof(k)); pline();
}
