#include "prlib.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

t_define	*define_data;

static char	*read_str_literal(char *p, bool is_str)
{
	if (is_str)
		printf("\"");
	else
		printf("'");
	p += 1;
	while (*p)
	{
		if (*p == '\\')
		{
			printf("\\%c", *(p + 1));
			p += 2;
		}
		else
		{
			printf("%c", *p);
			if ((*p == '"' && is_str)
				|| (*p == '\'' && !is_str))
				return (p + 1);
			p += 1;
		}
	}
	return (p);
}

static char	*dismiss_comment_line(char *p)
{
	char	*tmp;

	tmp = strchr(p, '\n');
	if (tmp == NULL)
		return (NULL);
	return (tmp);
}

static char	*dismiss_comment(char *p)
{
	char	*tmp;

	tmp = strstr(p, "*/");
	if (tmp == NULL)
		return (NULL);
	return (tmp + 2);
}

static char	*read_directive(char *p)
{
	p = skip_space(p);
	if (start_with(p, "include"))
		p = read_include_directive(p + 7);
	else if (start_with(p, "define"))
		p = read_define_directive(p + 6);
	else if (start_with(p, "ifdef"))
		p = read_ifdef_directive(p + 5, true);
	else if (start_with(p, "ifndef"))
		p = read_ifdef_directive(p + 6, false);
	else if (start_with(p, "end"))
	{
		// TODO ?
	}
	return p;
}

void	process(char *p)
{
	bool	can_use_directive;

	can_use_directive = true;
	while (p != NULL && *p)
	{
		if (start_with(p, "//"))
		{
			p = dismiss_comment_line(p);
			can_use_directive = true;
		}
		else if (start_with(p, "/*"))
			p = dismiss_comment(p);
		else if (*p == '\'' || *p == '"')
		{
			p = read_str_literal(p, *p == '"');
			can_use_directive = false;
		}
		else if (can_use_directive && *p == '#')
			p = read_directive(p + 1);
		else
		{
			if (*p == '\n') // 改行は\nだけ？
				can_use_directive = true;
			else if (!isspace(*p))
				can_use_directive = false;
			printf("%c", *p);
			p++;
		}
		// TODO directives
	}
}
