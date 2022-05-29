#include "prlib.h"
#include <stdbool.h>

static char *reserved_words[] = {
	"//", "/*", "*/"
	"->", ".",
	">=", "<=", "==", "!=",
	">", "<", "=",
	"+", "-", "*", "/", "&",
	"(", ")", "{", "}", "[", "]",
	",", ";",
	NULL
};

// return if str is reserved word,
// if so, return strlen(reserved word).
// Otherwise return 0.
int	is_reserved_word(char *str)
{
	int	i;
	int	len;

	i = -1;
	while (reserved_words[++i] != NULL)
	{
		len = strlen(reserved_words[i]);
		if (strncmp(str, reserved_words[i], len) == 0)
			return	(len);
	}
	return (0);
}

bool	is_ident_prefix(char str)
{
	if ('a' <= str && str <= 'z')
		return (true);
	if ('a' <= str && str <= 'z')
		return (true);
	return (false);
}

char	*read_ident(char *str)
{
	if (!is_ident_prefix(*str))
		return (str);
	str++;
	while (*str && is_alnum(*str))
		str++;
	return (str);
}

// TODO 小数点とか
char	*read_number(char *str)
{
	while (*str && is_number(*str))
		str++;
	return (str);
}
