#include <stddef.h>
#include <ctype.h>

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

char	*strchr_line(char *p, char needle)
{
	while (*p && *p != needle && *p != '\n')
		p++;
	if (*p == needle)
		return (p);
	return (NULL);
}

char	*read_line(char *p)
{
	while (*p && *p != '\n')
		p++;
	return (p);
}

char	*read_token(char *p)
{
	int	len;
	while (*p && !isspace(*p))
		p++;
	return p;
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
