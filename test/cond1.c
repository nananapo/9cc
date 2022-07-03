int dint(int a);

int main(void)
{
	dint(1 && 1);
	dint(0 && 1);
	dint(1 && 0);
	dint(0 && 0);

	dint(1 || 1);
	dint(0 || 1);
	dint(1 || 0);
	dint(0 || 0);

	dint(1 && 1 && 1);
	dint(1 && 1 && 0);
	dint(1 && 0 && 1);
	dint(1 && 0 && 0);

	dint(0 && 1 && 1);
	dint(0 && 1 && 0);
	dint(0 && 0 && 1);
	dint(0 && 0 && 0);

	dint(1 || 1 || 1);
	dint(1 || 1 || 0);
	dint(1 || 0 || 1);
	dint(1 || 0 || 0);

	dint(0 || 1 || 1);
	dint(0 || 1 || 0);
	dint(0 || 0 || 1);
	dint(0 || 0 || 0);
}
