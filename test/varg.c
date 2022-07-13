#include <stdio.h>
#include <stdarg.h>

int p(int a, int b, int c, int d, int e, int f);

int	test(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	vfprintf(stdout, fmt, ap);
}

int	main(void)
{
	test("HELLO %d %d\n", 1000, 2000);
}
