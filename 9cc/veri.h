#ifndef VERI_H
# define VERI_H

#include "9cc.h"
#include <stdbool.h>

typedef enum e_verikind
{
	VERI_NUM,
	VERI_ASSIGN,
	VERI_JUMP,
	VERI_FUNC_END,
	VERI_ADDRESS,
	VERI_RETURN,
	VERI_ADD,
	VERI_SUB,
	VERI_CAST,
	VERI_LOAD
}	t_verikind;

typedef struct s_veriproc
{
	t_verikind			kind;
	int					state_id;
	struct s_veriproc	*next;

	int					num;
	int					var_local_offset;
	t_type				*type;
	t_type				*cast_from;
}	t_veriproc;

#endif
