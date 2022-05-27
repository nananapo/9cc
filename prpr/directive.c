#include "prlib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern t_define	*define_data;

void	add_define(char *name, int name_len, char *replace, int repl_len)
{
	t_define	*tmp;

	tmp = (t_define *)malloc(sizeof(t_define));
	tmp->name = name;
	tmp->name_len = name_len;
	tmp->replace = replace;
	tmp->replace_len = repl_len;
	tmp->next = define_data;
	define_data = tmp;
	return ;
}

void	include_file(char *filename, int len)
{
	char	*str;
	
	str = read_file(strndup(filename, len));
	if (str == NULL)
		return ;
	process(str);
}

char	*read_include_directive(char *p)
{
	bool	is_standard_header;
	char	*tmp;

	p = skip_space(p);
	if (*p == '"')
		is_standard_header = false;
	else if (*p == '<')
		is_standard_header = true;
	else
		error_at(p, "invalid include header");
	p += 1;

	// read file name
	if (is_standard_header)
		tmp = strchr_line(p, '>');
	else
		tmp = strchr_line(p, '"');
	if (tmp == NULL)
		error_at(p, "missing terminating character");

	if (is_standard_header)
		//TODO
		tmp = tmp;
	else
		include_file(p, tmp - p);
	return tmp + 1;
}

char	*read_define_directive(char *p)
{
	return p;
}

char	*read_ifdef_directive(char *p, bool is_ifdef)
{
	return p;
}
