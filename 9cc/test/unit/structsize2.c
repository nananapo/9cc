int	pint(int n);
int	pline();

struct t1
{
	char a;
	char b;
	char c;
	char d;
	char e;
};

struct t2
{
	int a;
	char b;
};

struct t3
{
	int a;
	char b;
	int c;
};

struct t4
{
	int a;
	char b;
	int c;
	char d;
	int e;
};

struct t5
{
	int a;
	char b;
	char c;
	int d;
	int e;
};

int	main()
{
	struct t1 var1;
	struct t2 var2;
	struct t3 var3;
	struct t4 var4;
	struct t5 var5;

	pint(sizeof(var1)); pline();
	pint(sizeof(var2)); pline();
	pint(sizeof(var3)); pline();
	pint(sizeof(var4)); pline();
	pint(sizeof(var5)); pline();
}
