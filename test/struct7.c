int pchar(char n);
int pint(int n);
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
	char a;
	char b;
	char c;
	char d;
	char e;
};

struct t6
{
	char a;
	char b;
	char c;
	char d;
	char e;
	char f;
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
};

struct t8
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

struct t9
{
	int *a;
	char b;
};

int main()
{
	struct t1 v1;
	struct t2 v2;
	struct t3 v3;
	struct t4 v4;
	struct t5 v5;
	struct t6 v6;
	struct t7 v7;
	struct t8 v8;
	struct t9 v9;

	struct t1 v1_;
	struct t2 v2_;
	struct t3 v3_;
	struct t4 v4_;
	struct t5 v5_;
	struct t6 v6_;
	struct t7 v7_;
	struct t8 v8_;
	struct t9 v9_;

	v1.a = 100;

	v2.a = 101;
	v2.b = 102;

	v3.a = 103;
	v3.b = 104;
	v3.c = 105;

	v4.a = 106;
	v4.b = 107;
	v4.c = 108;
	v4.d = 109;

	v5.a = 110;
	v5.b = 111;
	v5.c = 112;
	v5.d = 113;
	v5.e = 114;

	v6.a = 115;
	v6.b = 116;
	v6.c = 117;
	v6.d = 118;
	v6.e = 119;
	v6.f = 120;

	v7.a = 121;
	v7.b = 122;
	v7.c = 123;
	v7.d = 124;
	v7.e = 125;
	v7.f = 126;
	v7.g = 99;

	v8.a = 48;
	v8.b = 49;
	v8.c = 50;
	v8.d = 51;
	v8.e = 52;
	v8.f = 53;
	v8.g = 54;
	v8.h = 55;

	int t;
	t = 1234567;
	v9.a = &t;
	v9.b = 56;

	pchar(v1.a); pline();

	pchar(v2.a); pline();
	pchar(v2.b); pline();

	pchar(v3.a); pline();
	pchar(v3.b); pline();
	pchar(v3.c); pline();

	pchar(v4.a); pline();
	pchar(v4.b); pline();
	pchar(v4.c); pline();
	pchar(v4.d); pline();

	pchar(v5.a); pline();
	pchar(v5.b); pline();
	pchar(v5.c); pline();
	pchar(v5.d); pline();
	pchar(v5.e); pline();

	pchar(v6.a); pline();
	pchar(v6.b); pline();
	pchar(v6.c); pline();
	pchar(v6.d); pline();
	pchar(v6.e); pline();
	pchar(v6.f); pline();

	pchar(v7.a); pline();
	pchar(v7.b); pline();
	pchar(v7.c); pline();
	pchar(v7.d); pline();
	pchar(v7.e); pline();
	pchar(v7.f); pline();
	pchar(v7.g); pline();

	pchar(v8.a); pline();
	pchar(v8.b); pline();
	pchar(v8.c); pline();
	pchar(v8.d); pline();
	pchar(v8.e); pline();
	pchar(v8.f); pline();
	pchar(v8.g); pline();
	pchar(v8.h); pline();

	pint(*v9.a); pline();
	pchar(v9.b); pline();

	v1_ = v1;
	v2_ = v2;
	v3_ = v3;
	v4_ = v4;
	v5_ = v5;
	v6_ = v6;
	v7_ = v7;
	v8_ = v8;
	v9_ = v9;

	pchar(v1_.a); pline();

	pchar(v2_.a); pline();
	pchar(v2_.b); pline();

	pchar(v3_.a); pline();
	pchar(v3_.b); pline();
	pchar(v3_.c); pline();

	pchar(v4_.a); pline();
	pchar(v4_.b); pline();
	pchar(v4_.c); pline();
	pchar(v4_.d); pline();

	pchar(v5_.a); pline();
	pchar(v5_.b); pline();
	pchar(v5_.c); pline();
	pchar(v5_.d); pline();
	pchar(v5_.e); pline();

	pchar(v6_.a); pline();
	pchar(v6_.b); pline();
	pchar(v6_.c); pline();
	pchar(v6_.d); pline();
	pchar(v6_.e); pline();
	pchar(v6_.f); pline();

	pchar(v7_.a); pline();
	pchar(v7_.b); pline();
	pchar(v7_.c); pline();
	pchar(v7_.d); pline();
	pchar(v7_.e); pline();
	pchar(v7_.f); pline();
	pchar(v7_.g); pline();

	pchar(v8_.a); pline();
	pchar(v8_.b); pline();
	pchar(v8_.c); pline();
	pchar(v8_.d); pline();
	pchar(v8_.e); pline();
	pchar(v8_.f); pline();
	pchar(v8_.g); pline();
	pchar(v8_.h); pline();

	pint(*v9_.a); pline();
	pchar(v9_.b); pline();
}
