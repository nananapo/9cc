#include "prlib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern t_define	*g_define_data;

void	add_define(char *name, int name_len, char *replace, int repl_len)
{
	t_define	*tmp;

	tmp = (t_define *)malloc(sizeof(t_define));
	tmp->name = name;
	tmp->name_len = name_len;
	tmp->replace = replace;
	tmp->replace_len = repl_len;
	tmp->next = g_define_data;
	g_define_data = tmp;
	return ;
}

char	*is_defined(char *name, int len)
{
	t_define	*tmp;

	tmp = g_define_data;
	while (tmp != NULL)
		if (tmp->name_len == len && strncmp(tmp->name, name, len) == 0)
			return (tmp);
	return (NULL);
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
	return (tmp + 1);
}

char	*read_define_directive(char *p)
{
	char	*name;
	int		name_len;
	char	*replace;
	int		repl_len;

	// read name
	p = skip_space(p);
	// TODO マクロなら括弧まで
	name = p;
	p = read_token(p);
	name_len = p - name;
	if (name_len == 0)
		error_at(p, "invalid define directive");

	// read macro
	p = skip_space(p);
	replace = p;
	p = read_line(p);
	repl_len = p - replace;

	add_define(name, name_len, replace, repl_len);
	return (p);
}

char	*read_ifdef_directive(char *p, bool is_ifdef)
{
	char	*name;

	p = skip_space(p);
	name = p;
	p = read_token(p);
	if (p - name == 0)
		error_ar(name, "invalid directive");
	if (is_defined(p, p - name) == is_ifdef)
	{
		// TODO endifかelseまで読む
		// elseはスキップ(出力しないだけで、解析はする)
	}
	else
	{
		// endifかelseまで読む
		// else以降を出力する
	}
	return (p);
}
