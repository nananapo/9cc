int	main()
{
	int *ptr;
	ptr = my_malloc_int(2);
	*ptr = 42;
	*(1 + ptr) = 24;
	pint(*ptr);
	pspace(1);
	pint(*(1 + ptr));
	return 0;
}
