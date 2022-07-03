int dint(int c);

int	main(void)
{
	int	a;

	a = 0;
	while (a++ < 100)
		break ;

	a = 0;
	while (a++ < 100)
	{
		dint(a);
		break ;
		dint(a);
	}

	a = 0;
	while (a++ < 100)
	{
		dint(a);
		break ;
		dint(a);
		break ;
	}

	a = 0;
	while (a++ < 100)
	{
		if (a == 50)
		{
			dint(a);
			break ;
		}
	}
}
