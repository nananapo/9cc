int	dint(int a);

int	main(void)
{
	int a = 1;
	int b = a + 1;
	int c = a + 2;
	int d = a + 3;
	int e = b + d;

	int f = a += 1;
	int g = b += 1;
	int h = c -= 100;

	dint(a);
	dint(b);
	dint(c);
	dint(d);
	dint(e);
	dint(f);
	dint(g);
	dint(h);
}
