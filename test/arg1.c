int	dint(int n);

int test1(int a, int b, int c, int d, int e, int f, int g)
{
	dint(a);
	dint(b);
	dint(c);
	dint(d);
	dint(e);
	dint(f);
	dint(g);
	return f + g;
}

int main()
{
	int result;

	result = test1(1,2,3,4,5,10,21);
	dint(result);

	return result;
}
