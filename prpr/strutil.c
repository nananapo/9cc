#include <stddef.h>
#include <ctype.h>
#include <stdbool.h>

int	start_with(char *haystack, char *needle)
{
	while (*needle && *haystack)
	{
		if (*needle != *haystack)
			return (0);
		needle++;
		haystack++;
	}
	return (*needle == '\0');
}

char	*skip_space(char *p)
{
	while (*p && (*p == ' ' || *p == '\t'))
		p++;
	return (p);
}

bool	is_number(char str)
{
	return ('0' <= str && str <= '9');
}

bool	is_alnum(char str)
{
	if ('a' <= str && str <= 'z')
		return (true);
	if ('a' <= str && str <= 'z')
		return (true);
	return (is_number(str));
}

bool	is_symbol(char str)
{
	return (str == '_');
}
