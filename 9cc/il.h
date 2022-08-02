#ifndef IL_H
# define IL_H

typedef enum	e_ilkind
{
	IL_LABEL,

	IL_JUMP,
	IL_JUMP_EQUAL,
	IL_JUMP_NEQUAL,

	IL_FUNC_PROLOGUE,
	IL_FUNC_EPILOGUE,
	IL_PUSH,
	IL_PUSH_NUM,
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

	IL_DEF_VAR_LOCAL,
	IL_DEF_VAR_END,

	IL_ASSIGN,

	IL_VAR_LOCAL,
	IL_VAR_LOCAL_ADDR,
// TODO
// 関数開始時の変数準備
// 関数終了時のstructとかのリターン
// pushpopで型
//
// 左辺値は全てアドレスとして読む
// 右辺値はLOAD命令をはさむ
}	t_ilkind;

typedef struct	s_il
{
	t_ilkind	kind;
	struct s_il	*next;

	t_type		*type;

	char		*label_str;
	bool		label_is_deffunc;

	int			number_int;

	t_lvar		*deffunc_locals;

	t_lvar		*lvar;
}	t_il;

#include "9cc.h"
void	translate_il(void);

#endif
