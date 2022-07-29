struct t1 {int a; char b; int c;};
int main()
{
	struct t1 s;
	struct t1 *p;
	p = &s;
	p->c = 2147483647;
	p->a = 2147483547;
	p->b = 15;
	return p->c - p->a + p->b;
}
