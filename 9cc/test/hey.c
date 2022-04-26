int	pcheck();
int pint(int i);
int pspace(int n);
int *my_malloc_int(int n);

int	main()
{
	int *ptr;

	ptr = my_malloc_int(2);

pcheck();

	*ptr = 42;

pcheck();

	*(ptr + 1) = 24;

pcheck();

	pint(*ptr);
	pspace(1);
	pint(*(ptr + 1));

	return 0;
}
