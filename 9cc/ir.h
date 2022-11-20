#ifndef IR_H
# define IR_H

# include <stdbool.h>
# include "9cc.h"
# include "list.h"

typedef enum e_irkind
{
	IR_FUNC,
	IR_NUMBER,
	IR_RETURN
}	t_irkind;

typedef enum e_ir_numtype
{
	IRNUM_INT,
	IRNUM_FLOAT
}	t_ir_numtype;

typedef struct s_ir_variable
{
	int		id;
	int		regid;
	//bool	is_phi_function;
	//t_list	*phi_ids;
}	t_ir_variable;

typedef struct s_ir_base
{
	t_irkind	kind;
}	t_ir_base;

typedef struct s_ir_stmt_base
{
	t_irkind				kind;
	t_ir_variable			*result;
	struct s_ir_stmt_base	*next_stmt;
}	t_ir_stmt_base;

typedef union u_ir
{
	struct s_ir_func
	{
		t_irkind		kind;

		char			*name;
		t_ir_stmt_base	*stmt;
		t_deffunc		*def;
	}	func;
	struct s_ir_return
	{
		t_irkind		kind;
		t_ir_variable	*result;
		t_ir_stmt_base	*next_stmt;

		bool			has_value;
	}	ret;
	struct s_ir_number
	{
		t_irkind		kind;
		t_ir_variable	*result;
		t_ir_stmt_base	*next_stmt;

		t_ir_numtype	numtype;
		int				value_int;
		float			value_float;
	}	number;
} t_ir;

//typedef struct s_ir_base		t_ir_base;
//typedef struct s_ir_stmt_base	t_ir_stmt_base;
typedef struct s_ir_func		t_ir_func;
typedef struct s_ir_return		t_ir_return;
typedef struct s_ir_number		t_ir_number;

void	translate_ir(void);

#endif
