int dint(int n);

int test(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j)
{
	dint(a);
	dint(b);
	dint(c);
	dint(d);
	dint(e);

	dint(f);
	dint(g);
	dint(h);
	dint(i);
	dint(j);
	return i;
}

int main()
{
	dint(test(1,3,5,7,9, 16,22,46,12,99));
}
