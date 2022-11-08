int dint(int a);

int	main(void)
{

	int k;

	if (1 != 1) k = 0;
	else if (2 == 2)
		dint(5);

	if (1 != 1) k = 0;
	else if (2 != 2) k = 0;
	else
		dint(9);

}
