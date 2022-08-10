int	pint(int a);
int pline();

struct t1
{
	char a;
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
	struct t6 f;
	struct t7 g;
	struct t8 h;
	struct t9 i;
	struct t10 j;
	struct t11 k;
	pint(sizeof(f)); pline();
	pint(sizeof(g)); pline();
	pint(sizeof(h)); pline();
	pint(sizeof(i)); pline();
	pint(sizeof(j)); pline();
	pint(sizeof(k)); pline();
}
