#include <stdbool.h>
#include <stdio.h>

typedef enum e_test
{
	E_1
}	testenum;

typedef struct s_test1
{
	testenum	a;
}	testst1;

typedef struct s_test2
{
	testenum	a;
	bool		b;
}	testst2;

typedef struct s_test3
{
	testenum	a;
	bool		b;
	bool		c;
}	testst3;

typedef struct s_test4
{
	testenum	a;
	bool		b;
	bool		c;
	void		*d;
}	testst4;

int main(void)
{
	printf("sizeof char %d\n", sizeof(char));
	printf("sizeof bool %d\n", sizeof(bool));
	printf("sizeof enum %d\n", sizeof(testenum));
	printf("sizeof st1 %d\n", sizeof(testst1));
	printf("sizeof st2 %d\n", sizeof(testst2));
	printf("sizeof st3 %d\n", sizeof(testst3));
	printf("sizeof st4 %d\n", sizeof(testst4));
}
