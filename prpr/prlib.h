#ifndef PRLIB_H
# define PRLIB_H

# include <stdbool.h>

typedef struct s_define
{
	char			*name;
	int				name_len;
	char			*replace;
	int				replace_len;
	struct s_define	*next;
}	t_define;

void	include_file(char *filename, int len);

char	*read_file(char *name);
void	process(char *str);

int		start_with(char *haystack, char *needle);
char	*skip_space(char *p);
char	*strchr_line(char *p, char needle);

char	*read_include_directive(char *p);
char	*read_define_directive(char *p);
char	*read_ifdef_directive(char *p, bool is_ifdef);

void	error(char *fmt, ...);
void	error_at(char *at, char *fmt, ...);

#endif
