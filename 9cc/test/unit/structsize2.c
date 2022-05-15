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

struct t6
{
	int a;
	char b;
};

struct t7
{
	char a;
	char b;
	char c;
	char d;
	char e;
	char f;
	char g;
	char h;
};

struct t8
{
	char a;
	char b;
	char c;
	char d;
	int	*p;
};

struct t9
{
	int z;
	char a;
	char b;
	char c;
	char d;
	char e;
};

struct t10
{
	int z;
	char a;
	char b;
	char c;
};

struct t11
{
	char a;
	char b;
	char c;
	char d;
	char e;
	char f;
	char g;
	char h;
	char i;
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
	struct t11 var11;

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
	pint(sizeof(var11)); pline();
}
