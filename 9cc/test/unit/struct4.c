int pline();
int pint(int a);
int pptr(int *a);

int main()
{
	int a[10];
	pptr(a); pline();
	pptr(a + 1); pline();
	pptr(&a[0]); pline();
	pptr(&a[1]); pline();
}
