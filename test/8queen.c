int	pint(int n);
int	pline();
int	pspace(int n);

int	N;
int	H[8];
int	pos[8];

int	abs(int n)
{
	if (n < 0)
		return -n;
	return n;
}

int can_put(int x, int y)
{
	int	i;
	int	j;

	pspace(x);
	pint(x);
	pspace(1);
	pint(9);
	pspace(2);
	if (H[y] != 0)
	{
		pline();
		return 0;
	}
	for (i = 0; i < x; i = i + 1)
	{
		pint(x);
		pspace(1);
		pint(i);
		pspace(1);
		pint(abs(x - i));
		pspace(1);
		pint(y);
		pspace(1);
		pint(pos[i]);
		pspace(1);
		pint(abs(y - pos[i]));
		pspace(1);
		pint(y - pos[i]);
		pspace(2);
		if (abs(x - i) == abs(y - pos[i]))
		{
			pline();
			return 0;
		}
	}
	pline();
	pint(x);
	pint(y);
	pline();
	return 1;
}

int	rec(int x)
{
	int	i;
	int result;

	pspace(x);
	pint(x);
	pline();

	if (x == N)
		return 1;
	result = 0;
	for (i = 0; i < N; i = i + 1)
	{
		pint(i);
		pline();
		if (can_put(x, i) == 1)
		{
			H[i] = 1;
			pos[x] = i;
			result = result + rec(x + 1);
			pos[x] = -1;
			H[i] = 0;
		}
	}

	pspace(x);
	pint(x);
	pspace(1);
	pint(result);
	pline();

	return result;
}

int	main()
{
	int res;

	N = 8;
	res = rec(0);
	pint(res);
	pline();

	return res;
}
