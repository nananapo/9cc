int pint(int n);
int pline();

int main()
{
	int a;
	int b;
	char c;
	char d;
	int e[2];
	char f[2];
	pint(&a - &b); pline();
	pint(&c - &d); pline();
	pint(&e[0] - &e[1]); pline();
	pint(&e[1] - &e[0]); pline();
	pint(&f[0] - &f[1]); pline();
	pint(&f[1] - &f[0]); pline();
}
