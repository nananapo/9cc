#include <stddef.h>

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
