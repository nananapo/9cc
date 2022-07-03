int dint(int c);

int main(void)
{
	dint(0);
	dint(1);
	dint(-1);
	dint(-42);
	dint(42);
	dint(-2147483648);
	dint(2147483647);

	dint(!0);
	dint(!1);
	dint(!-1);
	dint(!-42);
	dint(!42);
	dint(!-2147483648);
	dint(!2147483647);
	
	dint(!!0);
	dint(!!1);
	dint(!!-1);
	dint(!!-42);
	dint(!!42);
	dint(!!-2147483648);
	dint(!!2147483647);

	int i;

	for (i = -10; i < 10; i++)
		dint(i);

	for (i = -10; i < 10; i++)
		dint(!i);

	for (i = -10; i < 10; i++)
		dint(!!i);

	for (i = -10; i < 10; i++)
		dint(!!!i);

	for (i = -10; i < 10; i++)
		dint(!!!!-i);

	for (i = -10; i < 10; i++)
		dint(+!+!-!-!-i);
}
