int dint(int a);

int main(void)
{
	char a;
	int b;
	
	a = 1;
	b = 10000;
	
	a = 100000;
	b = 42;

	dint(a);
	dint(b);

	b = a;

	dint(a);
	dint(b);

	a = 1000;
	b = a;

	dint(a);
	dint(b);

	a = -100;
	b = a;

	dint(a);
	dint(b);

	b = -10000;
	a = b;

	dint(a);
	dint(b);
}
