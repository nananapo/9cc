int	dint(int c);

int	main(void)
{
	int	i;

	i = 0;
	do
		continue ;
	while (i++ < 100);

	i = 0;
	do
	{
		continue ;
	}
	while (i++ < 100);

	i = 0;
	do
	{
		dint(i);
		continue ;
	}
	while (i++ < 100);

	i = 0;
	do
	{
		continue ;
		dint(i);
	}
	while (i++ < 100);

	i = 0;
	do
	{
		dint(i);
		continue ;
		dint(i);
		continue ;
	}
	while (i++ < 100);

	i = 0;
	do
	{
		if (i == 0)
			continue ;
		else
		{
			dint(i);
			continue ;
		}
	}
	while (i++ < 100);
}
