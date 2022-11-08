#ifndef IR_H
# define IR_H

# include <stdbool.h>
# include "9cc.h"

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

typedef union u_ir
{
	struct s_ir_base
	{
		t_irkind	kind;
	}	base;
	struct s_ir_stmt_base
	{
		t_irkind				kind;
		struct s_ir_stmt_base	*next_stmt;
	}	stmtbase;
	struct s_ir_func
	{
		t_irkind				kind;

		char					*name;
		struct s_ir_stmt_base	*stmt;
		t_deffunc				*def;
	}	func;
	struct s_ir_return
	{
		t_irkind	kind;
		struct s_ir	*next_stmt;

		bool		has_value;
		union u_ir	*value;
	}	ret;
	struct s_ir_number
	{
		t_irkind		kind;

		t_ir_numtype	numtype;
		int				value_int;
		float			value_float;
	}	number;
} t_ir;

typedef struct s_ir_base		t_ir_base;
typedef struct s_ir_stmt_base	t_ir_stmt_base;
typedef struct s_ir_func		t_ir_func;
typedef struct s_ir_return		t_ir_return;
typedef struct s_ir_number		t_ir_number;

void	translate_ir(void);

#endif
