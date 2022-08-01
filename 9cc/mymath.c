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
