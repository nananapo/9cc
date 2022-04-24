#include <unistd.h>

static void	putchar(char c)
{
	write(1, &c, 1);
}

static int	recpint(int a)
{
	if (-9 <= a && a <= 9)
	{
		if (a < 0)
			a = -a;
		putchar('0' + a);
		return 0;
	}
	recpint(a / 10);
	recpint(a % 10);
	return 0;
}

int	pint(int a)
{
	if (a < 0)
		putchar('-');
	recpint(a);
	return 0;
}

int	pspace(int count)
{
	int	i;

	i = 0;
	while (i++ < count)
		putchar(' ');
	return 0;
}

int	pline()
{
	putchar('\n');
	return 0;
}
