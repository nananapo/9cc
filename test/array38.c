#include <stdio.h>

char *array[5][3]
= {
{ "A", "B", "C" },
{ "D", "E", "F" },
{ "G", "H", "I" },
{ "J", "K", "L" },
{ "M", "N", "O" }
};

int	main(void)
{
	int i;
	int j;

	char *a;
	char *b[5];

	printf("%d\n", sizeof(array));
	printf("%d\n", sizeof(a));
	printf("%d\n", sizeof(b));

	printf("%d\n", sizeof *array);

	printf("%s\n", array[0][0]);
	printf("%s\n", array[0][1]);
	printf("%s\n", array[0][2]);
	printf("%s\n", array[1][0]);
	printf("%s\n", array[1][1]);
	printf("%s\n", array[1][2]);
}
