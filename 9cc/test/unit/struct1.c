int pint(int a);
int pline();

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
	int a;
};

struct t4
{
	char	a;
	int		b;
};

struct t5
{
	char	a;
	int		b;
	int		c;
};

struct t6
{
	int		*a;
};

struct t7
{
	char	a;
	int		b;
	int		*c;
};

struct t8
{
	char	a;
	int		*b;
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

	pint(&var1 - &var1.a); pline();

	pint(&var2 - &var2.a); pline();
	pint(&var2 - &var2.b); pline();

	pint(&var3 - &var3.a); pline();

	pint(&var4 - &var4.a); pline();
	pint(&var4 - &var4.b); pline();

	pint(&var5 - &var5.a); pline();
	pint(&var5 - &var5.b); pline();
	pint(&var5 - &var5.c); pline();

	pint(&var6 - &var6.a); pline();

	pint(&var7 - &var7.a); pline();
	pint(&var7 - &var7.b); pline();
	pint(&var7 - &var7.c); pline();

	pint(&var8 - &var8.a); pline();
	pint(&var8 - &var8.b); pline();
}

