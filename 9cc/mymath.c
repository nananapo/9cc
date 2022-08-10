int	max(int a, int b)
{
	if (a > b)
		return (a);
	return (b);
}

int	min(int a, int b)
{
	if (a < b)
		return (a);
	return (b);
}

int	align_to(int n, int align)
{
	if (align == 0)
		return (n);
	return ((n + align - 1) / align * align);
}

int	mylog2(int n)
{
	int	r;

	r = 0;
	while (n > 1)
	{
		r += 1;
		n = n / 2;
	}
	return (r);
}
