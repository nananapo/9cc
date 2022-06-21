int dint(int c);

int	main(void)
{
	int	a;

	for(a = 0; a < 100; a++)
		break ;

	for (a = 0; a < 100; a++)
	{
		dint(a);
		break ;
		dint(a);
	}

	for (a = 0; a < 100; a++)
	{
		dint(a);
		break ;
		dint(a);
		break ;
	}

	for (a = 0; a < 100; a++)
	{
		if (a == 50)
		{
			dint(a);
			break ;
		}
	}
}
