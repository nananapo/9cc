#ifndef TEST
	#define TEST

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

#ifdef true
	YES
#endif

#ifndef false
	YES
#endif

#ifdef false
	NO
#else
	YES
#endif

	#include "test/t2.c"
#endif

