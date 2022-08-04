int	dint(int c);

int	main(void)
{
	int	*p;
	p = 10;
	dint(p);
	dint(p--);
	dint(p);
}
