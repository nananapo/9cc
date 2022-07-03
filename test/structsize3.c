int pint(int a);
int pline();

struct t1
{
	char	a;
	char	b;
	char	c;
};

struct t2
{
	char	a;
	char	*ptr;
};

struct t3
{
	char	a;
	int		b;
};

struct t4
{
	int		b;
	char	a;
};

struct t5
{
	int	a;
	int b;
	int c;
};

struct t6
{
	int a;
	int b;
	int c;
	char d;
};

struct t7
{
	
};

struct t8
{
	struct t7 b;
};

struct t9
{
	struct t8 a[10000];
};

int main()
{
	struct t1 z;
	struct t2 a;
	struct t3 b;
	struct t4 c;
	struct t5 d;
	struct t6 e;
	struct t7 f;
	struct t8 g;
	struct t9 h;
	pint(sizeof(z)); pline();
	pint(sizeof(a)); pline();
	pint(sizeof(b)); pline();
	pint(sizeof(c)); pline();
	pint(sizeof(d)); pline();
	pint(sizeof(e)); pline();
	pint(sizeof(f)); pline();
	pint(sizeof(g)); pline();
	pint(sizeof(h)); pline();
}
