#ifndef GEN_H
# define GEN_H
# include "9cc.h"

char	*get_str_literal_name(t_str_elem *elem);
void	gen(Node *node);
void	gen_defglobal(t_defvar *node);
void	gen_deffunc(t_deffunc *node);

#endif
