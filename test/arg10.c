int dint(int a);

int test(int a, int b, int c, int d, int e, int f, int g, int h)
{
	dint(a);
	dint(b);
	dint(c);
	dint(d);
	dint(e);
	dint(f);
	dint(g);
	dint(h);
	return 1;
}

int main()
{
	test(1, 2, 3, 4, 5, 6, 7, 8);
}
