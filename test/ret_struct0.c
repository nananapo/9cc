#include <stdio.h>

typedef struct s_struct3
{
	int a;
	int b;
	int c;
}	s_struct3;

s_struct3 retstruct3(void);


int	main(void)
{
	printf("%d\n", retstruct3().a);
	printf("%d\n", retstruct3().b);
	printf("%d\n", retstruct3().c);
	return 1;
}
