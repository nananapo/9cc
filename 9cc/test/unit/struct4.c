int pline();
int pint(int a);
int pptr(int *a);

int main()
{
	int a[10];
	pptr(a);
	pptr(a + 1);
	pptr(&a[0]);
	pptr(&a[1]);
}
