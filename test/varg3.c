#include <stdio.h>
#include <stdarg.h>

int p(int a, int b, int c, int d, int e, int f);

int	test(char *fmt, char *fmt2, ...)
{
	va_list ap;
	va_start(ap, fmt2);

	vfprintf(stdout, fmt, ap);
	vfprintf(stdout, fmt2, ap);
}

int	main(void)
{
	test("HELLO %d %d\n", "GOODBYE %d %d\n", 1000, 2000);
}
