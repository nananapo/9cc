int dint(int a);

int main(void)
{
	if (1 == 1)
		dint(1);
	else if (2 == 2)
		dint(2);
	else
		dint(3);

	if (1 != 1)
		dint(4);
	else if (2 == 2)
		dint(5);
	else
		dint(6);

	if (1 != 1)
		dint(7);
	else if (2 != 2)
		dint(8);
	else
		dint(9);

	if (1 != 1)
		dint(10);
	else if (2 != 2)
		dint(11);
	else if (3 != 3)
		dint(12);
	else
		dint(13);
}
