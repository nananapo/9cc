#include <stdio.h>

typedef struct s_struct2
{
	int a;
	int b;
}	s_struct2;

s_struct2 retstruct2_a(int b);

int	main(void)
{
	retstruct2_a(1000);
	return retstruct2_a(200).a;
}
