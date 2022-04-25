#include <stdio.h>

int main(void)
{
	int x = 42;
	int *p;

	printf("%p\n", &x);
	printf("%p\n", &x + 1);
	
}
