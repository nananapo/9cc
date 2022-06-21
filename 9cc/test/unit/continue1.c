int	dint(int c);

int	main(void)
{
	int	i;

	for (i = 0; i < 100; i++)
		continue ;

	for (i = 0; i < 100; i++)
	{
		continue ;
	}

	for (i = 0; i < 100; i++)
	{
		dint(i);
		continue ;
	}

	for (i = 0;i < 100; i++)
	{
		continue ;
		dint(i);
	}

	for (i = 0;i < 100; i++)
	{
		dint(i);
		continue ;
		dint(i);
		continue ;
	}

	for (i = 0;i < 100; i++)
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
