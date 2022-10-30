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
	VERI_ADDRESS
}	t_verikind;

typedef struct s_veriproc
{
	t_verikind			kind;
	int					state_id;
	struct s_veriproc	*next;

	int					num;
	int					var_local_offset;
	t_type				*type;

	bool				is_generated;
}	t_veriproc;

#endif
