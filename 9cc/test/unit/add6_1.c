int	dint(int n);

int add3(int x, int y, int z)
{
	dint(x);
	dint(y);
	dint(z);
	return x + y + z;
}

int add4(int x, int y, int z, int a)
{
	dint(x);
	dint(y);
	dint(z);
	dint(a);
	return x + y + z + a;
}

int add5(int x, int y, int z, int a, int b)
{
	dint(x);
	dint(y);
	dint(z);
	dint(a);
	dint(b);
	return x + y + z + a + b;
}

int add6(int x, int y, int z, int a, int b, int c)
{
	dint(x);
	dint(y);
	dint(z);
	dint(a);
	dint(b);
	dint(c);
	return x + y + z + a + b + c;
}

int main()
{
	dint(add3(1, 2, 3));
	dint(add4(1, 2, 3, 4));
	dint(add5(1, 2, 3, 4, 5));
	dint(add6(1, 2, 3, 4, 5, 6));
}
