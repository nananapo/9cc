int	dint(int a);

enum e1
{	
	TK1,
	TK2,
	TK3
};

int main(void)
{
	enum e1 a;

	a = TK1;
	dint(a);
	dint(a + 1);
	dint(a + 2);

	dint(TK1);
	dint(TK2);
	dint(TK3);
}
