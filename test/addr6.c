int main()
{
	int x;
	int *y;
	int **z;
	x = 3;
	y = &x;
	z = &y;
	**z = 42;
	return x;
}
