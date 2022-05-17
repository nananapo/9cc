#include <stdio.h>

struct t1
{
	char a;
};

struct t1 *fun(void)
{
	return NULL;
}

int main(void)
{
	return &fun()->a;
}
