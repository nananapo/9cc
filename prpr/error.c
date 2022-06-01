#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

# define ENV_DEV

void	error(char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	fprintf(stderr, "error: ");
	fprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void	error_at(char *at, char *fmt, ...)
{
	va_list	ap;
	char	*ret;

	va_start(ap, fmt);
	fprintf(stderr, "error: ");
	fprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	ret = strchr(at, '\n');
	if (ret == NULL)
		fprintf(stderr, "%s", at);
	else
		fprintf(stderr, "%s", strndup(at, ret - at));
	exit(1);
}

void	debug(char *fmt, ...)
{
#ifdef ENV_DEV
	va_list	ap;

	va_start(ap, fmt);
	fprintf(stdout, "DEBUG: ");
	fprintf(stdout, fmt, ap);
	fprintf(stdout, "\n");
#endif
}
