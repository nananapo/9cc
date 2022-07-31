#ifndef CHARUTIL_H
# define CHARUTIL_H

bool	issymbol(char c);
int		can_use_beginning_of_var(char c);
int		is_escapedchar(char c);
int 	char_to_int(char *p, int len);
void	put_str_literal(char *str, int len);

#endif
