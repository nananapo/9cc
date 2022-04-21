#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void	error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	fprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}
