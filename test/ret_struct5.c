#include <stdio.h>

typedef struct s_struct3
{
	int a;
	int b;
	int c;
}	s_struct3;

s_struct3 retstruct3_a(int a);

int	main(void)
{
	retstruct3_a(100).a;
	return retstruct3_a(100).a;
}
