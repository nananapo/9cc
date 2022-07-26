#include <stdio.h>
#include <stdbool.h>

int	main(void)
{
	printf("%d\n", true ? true : true);
	printf("%d\n", true ? true : false);
	printf("%d\n", true ? false : true);
	printf("%d\n", true ? false : false);

	printf("%d\n", false ? true : true);
	printf("%d\n", false ? true : false);
	printf("%d\n", false ? false : true);
	printf("%d\n", false ? false : false);

	int a;

	a = true ? 100 : 110;
	printf("a : %d\n", a);

	a = false ? 100 : 110;
	printf("a : %d\n", a);
}
