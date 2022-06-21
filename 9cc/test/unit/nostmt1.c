int	dint(int c);

int id(int c)
{
	dint(c);
	return (c);
}

int main(void)
{
	int i;

	i = 0;
	while (id(i++) < 100);

	for (i = 0; id(i) < 100; i++);
}
