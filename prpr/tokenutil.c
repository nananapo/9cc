#include "prlib.h"
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static char	*g_reserved_words[40] = {
	"//", "/*", "*/","...","->",
	".","--", "++",">=", "<=",

	"==", "!=","+=", "-=", "*=",
	"/=", "%=","||", "&&",">",

	"<", "=","+", "-", "*",
	"/", "%", "&", "!","(", "?",

	")", "{", "}", "[", "]",
	",", ";", ":", ""
};

// if str is reserved word, return strlen(reserved word).
// Otherwise return 0.
int	is_reserved_word(char *str)
{
	int	i;
	int	len;

	i = -1;
	while (strlen(g_reserved_words[++i]) != 0)
	{
		len = strlen(g_reserved_words[i]);
		if (strncmp(str, g_reserved_words[i], len) == 0)
			return (len);
	}
	return (0);
}

bool	is_ident_prefix(char str)
{
	if ('a' <= str && str <= 'z')
		return (true);
	if ('A' <= str && str <= 'Z')
		return (true);
	if (is_symbol(str))
		return (true);
	return (false);
}

char	*read_ident(char *str)
{
	if (!is_ident_prefix(*str))
		return (str);
	str++;
	while (*str && (isalnum(*str) || is_symbol(*str)))
		str++;
	return (str);
}

// TODO 小数点とか
char	*read_number(char *str)
{
	while (*str && isnumber(*str))
		str++;
	return (str);
}
