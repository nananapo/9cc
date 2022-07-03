int	dint(int a);

typedef enum e1
{	
	TK1,
	TK2,
	TK3
} e;

int main(void)
{
	e a;
	e *b;

	a = TK1;
	dint(a);

	b = &a;
	dint(*b);
	*b = TK2;

	dint(a);
}
