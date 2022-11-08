#ifndef CHARUTIL_H
# define CHARUTIL_H

bool	is_underscore(char c);
int		can_use_beginning_of_var(char c);
int		is_escapedchar(char c);
int 	char_to_int(char *p, int len);
char	*my_strcat(char *s1, char *s2);
#endif
