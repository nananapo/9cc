int dint(int c);

int main(void)
{
	int	i;

	i = 0;
	while (i++ < 100)
		dint(i);

	i = 0;
	while (++i < 100)
		dint(i);

	i = 0;
	while (++i < 100)
	{
		dint(i);
	}

	i = 0;
	while (i++ < 100)
	{
		dint(i);
	}

	i = 0;
	do
		dint(i);
	while (++i < 100);
	
	i = 0;
	do
		dint(i);
	while (i++ < 100);
	
	i = 0;
	do
	{
		dint(i);
	}
	while (++i < 100);

	i = 0;
	do
	{
		dint(i);
	}
	while (i++ < 100);
}
