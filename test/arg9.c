#define bool _Bool
#define true 1
#define false 0

int	dint(int a);
void	*malloc(int size);

typedef struct A
{
	int t;
}	A;

A	*test(bool a, int b, int c)
{
	dint(a);
	dint(b);
	dint(c);

	A *d;
	d = malloc(sizeof(A));
	d->t = a + b + c;
	return (d);
}

void	call(bool a, int b, int c)
{
	A	*result;
	result = test(a, b, c);

	dint(result->t);
}

int	main(void)
{
	call(true, 1000, 2000);
}
