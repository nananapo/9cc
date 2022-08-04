int dint(int c);

int	main(void)
{
	int	a;

	a = 0;
	do
		break ;
	while (a++ < 100);

	a = 0;
	do
	{
		dint(a);
		break ;
		dint(a);
	}
	while (a++ < 100);


	a = 0;
	do
	{
		dint(a);
		break ;
		dint(a);
		break ;
	}
	while (a++ < 100);

	a = 0;
	do
	{
		if (a == 50)
		{
			dint(a);
			break ;
		}
	}
	while (a++ < 100);
}
