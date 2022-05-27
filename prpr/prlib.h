#ifndef PRLIB_H
# define PRLIB_H

# include <stdbool.h>

char	*read_file(char *name);
int		process(char *str);

int		start_with(char *haystack, char *needle);
char	*skip_space(char *p);

char	*read_include_directive(char *p);
char	*read_define_directive(char *p);
char	*read_ifdef_directive(char *p, bool is_ifdef);

#endif
