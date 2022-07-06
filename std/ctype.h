#ifndef CTYPE_H
# define CTYPE_H

/*
 * C11のctype.h実装
 * Cロケールのみ対応 
 */

int	isalnum(int c);
int	isalpha(int c);
int	isblank(int c);
int	iscntrl(int c);
int	isdigit(int c);
int	isgraph(int c);
int	islower(int c);
int	isprint(int c);
int	ispunct(int c);
int	isspace(int c);
int	isupper(int c);
int	isxdigit(int c);
int	tolower(int c);
int	toupper(int c);

/*
int	isalnum(int c)
{
	return (isalpha(c) || isdigit(c));
}

int	isalpha(int c)
{
	return (isupper(c) || islower(c));
}

int	isblank(int c)
{
	return (c == ' ' || c == '\t');
}

int	iscntrl(int c)
{
	return ((0 <= c && c <= 31) || c == 127);
}

int	isdigit(int c)
{
	return ('0' <= c && c <= '9');
}

int	isgraph(int c)
{
	return (isprint(c) != 0 && c != ' ');
}

int	islower(int c)
{
	return ('a' <= c && c <= 'z');
}

int	isprint(int c)
{
	return (32 <= c && c <= 126);
}

int	ispunct(int c)
{
	return (isprint(c) && isspace(c) == 0 && isalnum(c) == 0);
}

int	isspace(int c)
{
	return (c == ' ' || c == '\f' || c == '\n'
		|| c == '\r' || c == '\t' || c == '\v');
}

int	isupper(int c)
{
	return ('A' <= c && c <= 'Z');
}

int	isxdigit(int c)
{
	return (('0' <= c && c <= '9')
		|| ('a' <= c && c <= 'f')
		|| ('A' <= c && c <= 'F'));
}

int	tolower(int c)
{
	if (isupper(c))
		return (c - 'A' + 'a');
	return (c);
}

int	toupper(int c)
{
	if (islower(c))
		return (c - 'a' + 'A');
	return (c);
}
*/

#endif
