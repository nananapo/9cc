#include <stdbool.h>

char	*read_include_directive(char *p)
{
	bool	is_standard_header;

	p = skip_space(p);
	if (*p == '"')
		is_standard_header = false;
	else if (*p == '<')
		is_standard_header = true;
	
		
	return p;
}

char	*read_define_directive(char *p)
{
	return p;
}

char	*read_ifdef_directive(char *p, bool is_ifdef)
{
	return p;
}
