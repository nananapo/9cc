int	dint(int c);

int	main(void)
{
	int c;

	c = 0;
	dint(c);
	dint(--c);
	dint(--c);
	dint(--c);
	dint(--c);
	
	int	*p;
	int	*p2;

	p = &c;
	p2 = &c;
	dint(*p);
	dint(*p2);
	
	c--;

	dint(*p);
	dint(*p2);

	dint(--p - p2);
	dint(--p - p2);
	dint(--p - p2);
	dint(--p - p2);

	int	a[100];
	a[0] = 100;
	a[99] = 2147483647;
	for (c = 1; c < 99; ++c)
		a[c] = c * 10;
	for (c = 0; c < 100; ++c)
		dint(a[c]);
}
