#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool	is_underscore(char c)
{
	return (c == '_');
}

int	can_use_beginning_of_var(char c)
{
	return (isalpha(c) || is_underscore(c));
}

int	is_escapedchar(char c)
{
	return (c == '"' || c == 'a' || c == 'b' || c == 'f' || c == 'n' || c == 'e' || c == 'x' || c == 'r' || c == 't' || c == 'v' || c == '0' || c == '\'' || c == '\\');
}

// lenはstrlen_actualを使う
int char_to_int(char *p, int len)
{
	if (len == 1)
		return (*p);
	p++;

	switch (*p)
	{
		case '"':
		case '\'':
			return (*p);
		case 'a':
			return '\a';
		case 'b':
			return '\b';
		case 'f':
			return '\f';
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		case 'v':
			return '\v';
		case '0':
			return '\0';
		case '\\':
			return '\\';
	}
	return -1;
}

char	*my_strcat(char *a, char *b)
{
	char	*res;

	res = calloc(1, sizeof(char) * (strlen(a) + strlen(b) + 1));
	strcat(res, a);
	strcat(res, b);
	return (res);
}
