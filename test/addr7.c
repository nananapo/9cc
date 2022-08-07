int dint(int x);

int swap(int *x, int *y)
{
	int tmp;
	tmp = *x;
	*x = *y;
	*y = tmp;
	return 0;
}

int main()
{
	int x;
	int y;
	x = 24;
	y = 42;
	dint(x);
	dint(y);
	swap(&x, &y);
	dint(x);
	dint(y);
	return 0;
}
