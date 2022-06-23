#include "9cc.h"
#include <ctype.h>

int	can_use_beginning_of_var(char c)
{
	return isalpha(c) || (c == '_');
}

int	is_escapedchar(char c)
{
	return (c == '"' || c == 'a' || c == 'b' || c == 'f' || c == 'n'
		|| c == 'r' || c == 't' || c == 'v' || c == '0' || c == '\'' || c == '\\');
}

int get_char_to_int(char *p, int len)
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
		default:
			error("不明なエスケープシーケンスです %c", *p);
	}
	return -1;
}
