#include "prlib.h"
#include <stddef.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

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
					break ;
				case 'a':
					c = '\a';
					break ;
				case 'b':
					c = '\b';
					break ;
				case 'f':
					c =  '\f';
					break ;
				case 'n':
					c =  '\n';
					break ;
				case 'r':
					c = '\r';
					break ;
				case 'v':
					c = '\v';
					break ;
				case '0':
					c = '\0';
					break ;
				case '\\':
					c = '\\';
					break ;
				default:
					error("不明なエスケープシーケンスです %c", str[i]);
					c = 0;
					break ;
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
