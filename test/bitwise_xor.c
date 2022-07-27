#include <stdio.h>

int	main(void)
{
	int	a;
	int	b;

	a = 1;
	b = 2;
	printf("%d\n", a ^ b);
	a = 1000;
	b = 2211;
	printf("%d\n", a ^ b);
	a = 11345645;
	b = 1231414;
	printf("%d\n", a ^ b);
	a = 12314214;
	b = 434444;
	printf("%d\n", a ^ b);
	a = 9786;
	b = 1288;
	printf("%d\n", a ^ b);

	int	c;

	a = 9782226;
	b = 1121288;
	c = 1345367;
	printf("%d\n", a ^ b ^ c);
	a = 1324346;
	b = 1543435;
	c = 7654546;
	printf("%d\n", a ^ b ^ c);
	a = 8675645;
	b = 5479883;
	c = 8907342;
	printf("%d\n", a ^ b ^ c);
	a = 9987132;
	b = 876543;
	c = 9083891;
	printf("%d\n", a ^ b ^ c);
}
