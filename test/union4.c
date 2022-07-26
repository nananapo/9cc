#include <stdio.h>

union u_z { union u_y { int a; } a; union u_z *p; };

int	main(void)
{
	 union	u_a { int a;};
	 union	u_b { int a;} c;

	printf("%lu\n", sizeof(union u_a));
	printf("%lu\n", sizeof(union u_b));
	printf("%lu\n", sizeof(union u_y));
	printf("%lu\n", sizeof(union u_z));
}
