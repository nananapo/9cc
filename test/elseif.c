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
		dint(1);
	else if (2 == 2)
		dint(2);
	else
		dint(3);

	if (1 != 1)
		dint(1);
	else if (2 != 2)
		dint(2);
	else
		dint(3);
	if (1 != 1)
		dint(1);
	else if (2 != 2)
		dint(2);
	else if (3 != 3)
		dint(2);
	else
		dint(3);
}
