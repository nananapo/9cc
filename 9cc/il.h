#ifndef IL_H
# define IL_H
# include "9cc.h"

// va_startでは何もpushしない
typedef enum e_ilkind
{
	IL_LABEL,
	IL_JUMP,
	IL_JUMP_EQUAL,
	IL_JUMP_NEQUAL,
	IL_FUNC_PROLOGUE,

	IL_FUNC_EPILOGUE,
	IL_DEF_VAR_LOCAL,
	IL_DEF_VAR_END,
	IL_PUSH_AGAIN,
	IL_PUSH_NUM,

	IL_STACK_SWAP,
	IL_POP,
	IL_ADD,
	IL_SUB,
	IL_MUL,

	IL_DIV,
	IL_MOD,
	IL_EQUAL,
	IL_NEQUAL,
	IL_LESS,

	IL_LESSEQ,
	IL_BITWISE_AND,
	IL_BITWISE_OR,
	IL_BITWISE_XOR,
	IL_BITWISE_NOT,

	IL_SHIFT_LEFT,
	IL_SHIFT_RIGHT,
	IL_ASSIGN,
	IL_VAR_LOCAL,
	IL_VAR_LOCAL_ADDR,

	IL_VAR_GLOBAL,
	IL_VAR_GLOBAL_ADDR,
	IL_MEMBER,
	IL_MEMBER_ADDR,
	IL_MEMBER_PTR,

	IL_MEMBER_PTR_ADDR,
	IL_STR_LIT,
	IL_CALL_START,
	IL_CALL_ADD_ARG,
	IL_CALL_EXEC,

	IL_MACRO_VASTART,
	IL_CAST,
	IL_LOAD
}	t_ilkind;

typedef struct s_il
{
	t_ilkind	kind;
	struct s_il	*next;

	t_type		*type;

	char		*label_str;
	bool		label_is_deffunc;
	bool		label_is_static_func;

	int			number_int;

	t_deffunc	*deffunc_def;
	t_lvar		*deffunc_locals;

	t_lvar		*var_local;
	t_defvar	*var_global;

	t_str_elem	*def_str;

	t_deffunc	*funccall_callee;
	//t_deffunc	*funccall_caller;

	t_type		*cast_from;
	t_type		*cast_to;

	t_type		*stack_up;
	t_type		*stack_down;

	t_member	*member;
}	t_il;

void	translate_il(void);

#endif
