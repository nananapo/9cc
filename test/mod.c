int	pint(int c);
int	dint(int c);
int	pspace(int c);

int	main(void)
{
	int	i;

	for(i = -18; i < 18; i = i + 1)
	{
		pint(i);
		pspace(1);
		dint(i % 17);
	}
}
