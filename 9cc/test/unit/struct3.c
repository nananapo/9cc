#include <stdio.h>

struct t1
{

};

int main()
{

	struct t1 c[2];
	printf("%p\n", &c[0]);
	printf("%p\n", &c[1]);
	printf("%lu\n", &c[1] - &c[0]);
}

/*
	int a[2];
	char b[2];
	printf("%ld\n", &a[0] - &a[1]);
	printf("%ld\n", &a[1] - &a[0]);
	printf("%ld\n", &b[0] - &b[1]);
	printf("%ld\n", &b[1] - &b[0]);
}
*/
