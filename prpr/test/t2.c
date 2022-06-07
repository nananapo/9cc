#define true 1
#define false 0
#define bool _Bool

bool	is_even(int x)
{
	if (x % 2 == 0)
		return (true);
	return (false);
}

int	main(void)
{
	#undef false
	return false;
}
