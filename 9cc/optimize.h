#ifndef OPTIMIZE_H
# define OPTIMIZE_H
# include "9cc.h"
# include <stdbool.h>

typedef struct	s_basicblock
{
	t_il				*start;
	t_il				*end;
	struct s_basicblock	*next;
	struct s_basicblock	*next_if;
	int					uniqueid;

	bool				is_constructed;
	bool				il_generated;

	struct s_basicblock	*mark_block;
}	t_basicblock;

typedef struct	s_pair_ilblock
{
	t_il					*il;
	t_basicblock			*block;
	struct s_pair_ilblock	*next;
}	t_pair_ilblock;

typedef struct s_label_pair
{
	char				*from;
	char				*to;
	struct s_label_pair	*next;
}	t_label_pair;

#endif
