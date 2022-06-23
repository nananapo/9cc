int	dint(int c);

int	main(void)
{
	int i;

	for (i = 0; i < 100; i++)
	{
		switch (i)
		{
			case 1:
				dint(1);
			case 2:
				dint(2);
				break;
			default:
				dint(i);
		}
	}

	for (i = 0; i < 100; i++)
	{
		switch (i)
		{
			default:
				dint(i);
			case 1:
				dint(1);
			case 2:
				dint(2);
				break;
		}
	}
}
