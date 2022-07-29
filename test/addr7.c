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
	swap(&x, &y);
	pint(x);
	pspace(1);
	pint(y);
	return 0;
}
