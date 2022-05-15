struct	s_test
{
	int		test1;
	int		test2;
	char	c1;
	char	c2;
	int		test3;
	int		*a;
};

int	main()
{
	struct s_test a;
	a.test1 = 123456;
	a.test2 = 987655;
	a.c1	= 'k';
	a.c2	= 'j';
	a.test3 = 999;
	a.a = 0;
}
