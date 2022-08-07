#include <stdio.h>

typedef struct s_struct2
{
	int a;
	int b;
}	s_struct2;

s_struct2 retstruct2(void);

int	main(void)
{
	printf("%d\n", retstruct2().a);
	printf("%d\n", retstruct2().b);
	return 1;
}
