int pint(int n);
int pline();

int N;
int pos[10];

// 絶対値を返す
int abs(int n)
{
	if (n < 0)
		return - n;
	return n;
}

// (x, y) : (i, y)にqueenを置けるかどうかを返す
int can_put(int i, int y)
{
	int j;
	for (j = 0; j < i; j = j + 1)
	{
		if (pos[j] == y)
			return 0;
		if (abs(i - j) == abs(y - pos[j]))
			return 0;
	}
	return 1;
}

int rec(int i)
{
	int answer;
	int y;

	if (i == N)
		return 1;

	answer = 0;
	for (y = 0; y < N; y = y + 1)
	{
		if (can_put(i, y))
		{
			pos[i] = y;
			answer = answer + rec(i + 1);
		}
	}
	return answer;
}

int main()
{
	N = 9;
	pint(rec(0));
	pline();
}
