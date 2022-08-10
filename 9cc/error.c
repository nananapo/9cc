#include "9cc.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

//#define DEBUG

extern char	*g_user_input;

void	error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	fprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void	debug(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

#ifdef DEBUG
	fprintf(stdout, "#");
	vfprintf(stdout, fmt, ap);
	fprintf(stdout, "\n");
#endif
}


static int	count_line(char *start, char *end)
{
	int	i;

	i = 1;
	while (start != end)
	{
		if (*start == '\n')
			i++;
		start++;
	}
	return (i);
}

void	error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	char	*mystart;
	char	*ret;

	va_start(ap, fmt);

	mystart = loc;
	while (mystart != g_user_input && *(mystart - 1) != '\n')
		mystart--;

	int pos = loc - mystart;

	fprintf(stderr, "%d:%d:\n", count_line(g_user_input, loc), pos);

	ret = strchr(mystart, '\n');
	if (ret == NULL)
		fprintf(stderr, "%s\n", mystart);
	else
		fprintf(stderr, "%s\n", my_strndup(mystart, ret - mystart));

	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

