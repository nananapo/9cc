int	main()
{
	int *ptr;
	ptr = my_malloc_int(2);
	*ptr = 42;
	*(ptr + 1) = 24;
	pint(*ptr);
	pspace(1);
	pint(*(ptr + 1));
	return 0;
}
