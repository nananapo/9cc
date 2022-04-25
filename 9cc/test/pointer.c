#include <stdio.h>

int main(void)
{
	int x = 42;
	int y = 42;
	printf("%d\n", &x > &y);
}
