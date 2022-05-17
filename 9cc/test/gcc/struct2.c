#include <stdio.h>

/*
 * これはだめ
 */
int main()
{
	int *a;

	a = NULL;
	printf("%ld\n", &(a + 1));
}
