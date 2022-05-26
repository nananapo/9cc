#include "9cc.h"

int is_uppercase(char c)
{
	return ('A' <= c && c <= 'Z');
}

int is_lowercase(char c)
{
	return ('a' <= c && c <= 'z');
}

int	is_number(char c)
{
	return ('0' <= c && c <= '9');
}

int	is_alphabet(char c)
{
	return is_uppercase(c) || is_lowercase(c);
}

int	can_use_beginning_of_var(char c)
{
	return is_alphabet(c) || (c == '_');
}

int	is_alnum(char c)
{
	return can_use_beginning_of_var(c) || is_number(c);
}

int	is_escapedchar(char c)
{
	return (c == '"' || c == 'a' || c == 'b' || c == 'f' || c == 'n'
		|| c == 'r' || c == 'v' || c == '0' || c == '\'' || c == '\\');
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
