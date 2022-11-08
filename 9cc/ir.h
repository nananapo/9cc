#ifdef IR_H
# define IR_H

typedef enum e_irkind
{
	IR_FUNC,
	
} t_irkind;


typedef union u_ir
{
	struct s_ir_base
	{
		t_irkind kind;
	} base;
	struct s_ir_func
	{
		t_irkind	kind;
		union u_ir	*code;
	} func;	
} t_ir;

#endif
