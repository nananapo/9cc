int	dint(int c);

int	main(void)
{
	int	i;

	i = 0;

	dint(i);
	dint(i += 1);
	dint(i -= 1);
	dint(i += 10);
	dint(i *= 5);
	dint(i /= 25);
	dint(i += 3);
	dint(i += i);
}
