struct t1 {int a; char b; int c;};
int main()
{
	struct t1 s;
	struct t1 *p;
	p = &s;
	p->c = 65535;
	p->a = 65435;
	p->b = 12;
	return p->c - p->a + p->b;
}
