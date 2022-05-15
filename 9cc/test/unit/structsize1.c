int	pint(int n);
int	pline();

struct t1
{
	char a;
};

struct t2
{
	char a;
	char b;
};

struct t3
{
	char a;
	char b;
	char c;
};

struct t4
{
	char a;
	char b;
	char c;
	char d;
};

struct t5
{
	int a;
};

struct t6
{
	char	a;
	int		b;
};

struct t7
{
	char	a;
	int		b;
	int		c;
};

struct t8
{
	int		*b;
};

struct t9
{
	char	a;
	int		b;
	int		*c;
};

struct t10
{
	char	a;
	int		*c;
};

int	main()
{
	struct t1 var1;
	struct t2 var2;
	struct t3 var3;
	struct t4 var4;
	struct t5 var5;
	struct t6 var6;
	struct t7 var7;
	struct t8 var8;
	struct t9 var9;
	struct t10 var10;

	pint(sizeof(var1)); pline();
	pint(sizeof(var2)); pline();
	pint(sizeof(var3)); pline();
	pint(sizeof(var4)); pline();
	pint(sizeof(var5)); pline();
	pint(sizeof(var6)); pline();
	pint(sizeof(var7)); pline();
	pint(sizeof(var8)); pline();
	pint(sizeof(var9)); pline();
	pint(sizeof(var10)); pline();
}
