int	my_print(char *str);

int main(void)
{
	int	i;

	for (i = 0; i < 100; i++)
	{
		switch (i)
		{
			case 0:
				my_print("oh....\n");
				break;
			case 1:
				my_print("aaa\n");
				break;
			case 2:
			case 3:
				my_print("Hello\n");
				break;
			case 4:
				my_print("Hey!\n");
			case 5:
				my_print("Hola\n");
				break;
		}
	}
}
