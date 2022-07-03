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

	a = TK1;
	if (a == TK1)
		dint(0);
	if (a == TK2)
		dint(1);
	if (a == TK3)
		dint(2);
}
