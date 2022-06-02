#include "prlib.h"
#include <stddef.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

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
	if ('A' <= str && str <= 'Z')
		return (true);
	return (is_number(str));
}

bool	is_symbol(char str)
{
	return (str == '_');
}

void	add_str_elem(StrElem **list, char *str)
{
	StrElem	*elem;
	StrElem	*tmp;

	elem = malloc(sizeof(StrElem));
	elem->str = str;
	elem->next = NULL;
	if (*list == NULL)
		*list = elem;
	else
	{
		tmp = *list;
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = elem;
	}
}

char	*strlit_to_str(char *str, int len)
{
	int		i;
	int		bi;
	char	*buf;
	char	c;

	i = -1;
	bi = 0;
	buf = calloc(len + 1, sizeof(char));
	while (++i < len)
	{
		if (str[i] == '\\')
		{
			switch (str[++i])
			{		
				case '"':
				case '\'':
					c = str[i];
				case 'a':
					c = '\a';
				case 'b':
					c = '\b';
				case 'f':
					c =  '\f';
				case 'n':
					c =  '\n';
				case 'r':
					c = '\r';
				case 'v':
					c = '\v';
				case '0':
					c = '\0';
				case '\\':
					c = '\\';
				default:
					error("不明なエスケープシーケンスです %c", str[i]);
			}
		}
		else
		{
			c = str[i];
		}
		buf[bi++] = c;
	}
	return (buf);
}
