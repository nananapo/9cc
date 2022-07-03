int	dint(int c);

int	main(void)
{
	int	i;

	i = 0;
	while (i++ < 100)
		continue ;

	i = 0;
	while (i++ < 100)
	{
		continue ;
	}

	i = 0;
	while (i++ < 100)
	{
		dint(i);
		continue ;
	}

	i = 0;
	while (i++ < 100)
	{
		continue ;
		dint(i);
	}

	i = 0;
	while (i++ < 100)
	{
		dint(i);
		continue ;
		dint(i);
		continue ;
	}

	i = 0;
	while (i++ < 100)
	{
		if (i == 0)
			continue ;
		else
		{
			dint(i);
			continue ;
		}
	}
}
