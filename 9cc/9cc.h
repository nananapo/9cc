#ifndef NINECC_H
# define NINECC_H

#include <stdbool.h>
#include "list.h"

# define ASM_MOV "mov"
# define ASM_PUSH "push"
# define ASM_LEA "lea"

// 64
# define RAX "rax"
# define RDI "rdi"
# define RSI "rsi"
# define RBP "rbp"
# define RSP "rsp"
# define R10 "r10"
# define R11 "r11"
# define RDX "rdx"

// 32
# define EAX "eax"
# define EDI "edi"
# define ESI "esi"
# define R11D "r11d"

// 16
# define AX "ax"
# define SI "si"
# define R11W "r11w"

// 8
# define AL "al"
# define DIL "dil"
# define SIL "sil"
# define CL "cl"
# define R11B "r11b"

#define BYTE_PTR "byte ptr"
# define WORD_PTR "word ptr"
# define DWORD_PTR "dword ptr"

# define ARGREG_SIZE 6

typedef enum e_tokenkind
{
	TK_RESERVED,
	TK_IDENT,
	TK_NUM,
	TK_RETURN,
	TK_IF,

	TK_ELSE,
	TK_WHILE,
	TK_DO,
	TK_FOR,
	TK_BREAK,

	TK_SWITCH,
	TK_CASE,
	TK_DEFAULT,

	TK_CONTINUE,
	TK_STR_LITERAL,
	TK_CHAR_LITERAL,
	TK_EOF,
	TK_SIZEOF,

	TK_INT,
	TK_CHAR,
 	TK__BOOL,
	TK_VOID,
	TK_STRUCT,
	TK_ENUM,
	TK_UNION,

	TK_STATIC,
	TK_TYPEDEF,
	TK_EXTERN,
 	TK_INLINE
} t_tokenkind;

typedef struct s_token
{
	t_tokenkind		kind;
	struct s_token	*next;

	int				val;			// number
	char			*str;			// token str
	int				len;			// length
	int				strlen_actual;	// charlit
} t_token;

typedef enum e_nodekind
{
	ND_NONE, // 式ではないものはND_NONEにする
	ND_ANALYZE_VAR,
	ND_CALL,
	ND_BLOCK,
	ND_NUM,

	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_MOD,



	ND_COMP_ADD,
	ND_COMP_SUB,
	ND_COMP_MUL,
	ND_COMP_DIV,
	ND_COMP_MOD,

	ND_EQUAL,
	ND_NEQUAL,
	ND_LESS,
	ND_LESSEQ,
	ND_ASSIGN,



	ND_VAR_DEF,
	ND_VAR_LOCAL,
 	ND_VAR_GLOBAL,
	ND_SHIFT_LEFT,
	ND_SHIFT_RIGHT,

	ND_BITWISE_AND,
	ND_BITWISE_OR,
	ND_BITWISE_NOT,
	ND_BITWISE_XOR,
	ND_COND_OP,



	ND_COND_AND,
	ND_COND_OR,
	ND_RETURN,
	ND_IF,
	ND_WHILE,

	ND_DOWHILE,
	ND_FOR,
	ND_SWITCH,
	ND_CASE,
	ND_ADDR,



	ND_DEREF,
	ND_STR_LITERAL,
	ND_MEMBER_VALUE, // .
	ND_MEMBER_PTR_VALUE, // ->
	ND_CAST,

	ND_PARENTHESES,
	ND_BREAK,
	ND_CONTINUE,
	ND_DEFAULT,
	ND_SIZEOF,

	ND_ADD_UNARY,
	ND_SUB_UNARY,
	ND_DEF_VAR,
	
	ND_VAR_LOCAL_ADDR,
	ND_VAR_GLOBAL_ADDR,
	ND_DEREF_ADDR,
	ND_MEMBER_VALUE_ADDR,
	ND_MEMBER_PTR_VALUE_ADDR
} t_nodekind;

typedef enum e_typekind
{
	TY_INT,
	TY_CHAR,
	TY_PTR,
	TY_ARRAY,
	TY_STRUCT,
	TY_ENUM,
	TY_UNION,
	TY_BOOL,
	TY_VOID
} t_typekind;

typedef struct s_defenum
{
	char	*name;
	int		name_len;

	char	*kinds[1000];
	int		kind_len;
}	t_defenum;

typedef struct s_type
{
	t_typekind		ty;
	struct s_type	*ptr_to;

	int 			array_size;

	struct s_defstruct
	{
		char				*name;
		int					name_len;
		int					mem_size; // -1ならまだ定義されていない
		struct s_member
		{
			struct s_type	*type;
			char			*name;
			int				name_len;
			int				offset;
			struct s_member	*next;
		} *members;
	}	*strct;

	t_defenum			*enm;

	struct s_defunion
	{
		char			*name;
		int				name_len;
		int				mem_size;
		struct s_member	*members;
	}	*unon;
} t_type;

typedef struct s_defstruct	t_defstruct;
typedef struct s_defunion	t_defunion;
typedef struct s_member		t_member;

typedef struct	s_switchcase
{
	int					value;
	int					label;
	struct s_switchcase	*next;
}	t_switchcase;

typedef struct s_labelstack
{
	bool			isswitch;

	int				startLabel;
	int				endLabel;
	int				defaultLabel;

	t_type			*type;
	t_switchcase	*cases;
}	t_labelstack;

typedef struct s_lvar
{
	struct s_lvar	*next;

	char		*name;
	int			name_len;
	int			offset;

	bool		is_arg;
	int			arg_regindex;
	bool		is_dummy;

	t_type		*type;
} t_lvar;

typedef struct s_str_literal_elem
{
	char	*str;
	int		len;
	int		index;
}	t_str_elem;

typedef struct s_node
{
	t_nodekind		kind;
	struct s_node	*lhs;
	struct s_node	*rhs;

	t_type			*type;

	t_lvar			*lvar; // local var
	struct s_node	*lvar_assign; // type ident = expr;

	t_str_elem		*def_str;
struct s_defvar
{
	char			*name;
	int				name_len;

	t_type			*type;

	// for global variable
	bool			is_extern;
	bool			is_static;
	struct s_node	*assign;
}	*var_global;

	struct s_node	*global_assign_next;

	// num
	int				val;

	// else of if
	struct s_node	*elsif;
	struct s_node	*els;

	struct s_node	*for_expr[3]; // for (0; 1; 2)

	// call
struct s_deffunc
{
	char		*name;
	int			name_len;

	t_type		*type_return;

	int			argcount;

	char		*argument_names[20];
	int			argument_name_lens[20];
	t_type		*type_arguments[20];

	bool		is_static;
	bool		is_prototype;
	bool		is_zero_argument;		// func(void)
	bool		is_variable_argument;	// func(, ...)

	t_lvar		*locals;
	struct s_node	*stmt;
}	*funcdef;
	int				funccall_argcount;
	struct s_node	*funccall_args[20];
	t_lvar			*funccall_argdefs[20];

	// 返り値がMEMORYかstructな関数を呼んだ時に結果を入れる場所
	t_lvar			*call_mem_stack;

	t_member		*elem;

	// valとlabelでswicth-case
	t_switchcase	*case_label;
	bool			switch_has_default; // TODO labelstackに移動する

	t_labelstack	*block_sbdata;




	// analyze
	bool		is_analyzed;
	char		*analyze_source;

	char		*analyze_var_name;
	int			analyze_var_name_len;

	char		*analyze_funccall_name;
	int			analyze_funccall_name_len;

	char		*analyze_member_name;
	int			analyze_member_name_len;
}	t_node;

typedef struct s_deffunc t_deffunc;
typedef struct s_defvar	t_defvar;

typedef struct s_typedefpair
{
	char	*name;
	int		name_len;
	t_type	*type;
}	t_typedefpair;


void	debug(char *fmt, ...);

char	*read_file(char	*name);

int		align_to(int a, int to);

void	error(char *fmt, ...);
void	error_at(char *loc, char *fmt, ...);

t_token	*tokenize(char *p);

t_node	*new_node(t_nodekind kind, t_node *lhs, t_node *rhs);
t_node	*new_node_num(int val);

// t_type
t_type	*new_primitive_type(t_typekind pri);
int		get_type_size(t_type *type);
t_type	*new_type_ptr_to(t_type *ptr_to);
t_type	*new_type_array(t_type *ptr_to);
t_type	*new_struct_type(char *name, int len);
t_type	*new_enum_type(char *name, int len);
t_type	*new_union_type(char *name, int len);
bool	is_integer_type(t_type *type);
bool	is_pointer_type(t_type *type);
bool	is_declarable_type(t_type *type);
bool	can_compared(t_type *l, t_type *r, t_type **lt, t_type **rt);
bool	type_equal(t_type *t1, t_type *t2);
t_member	*get_member_by_name(t_type *type, char *name, int len);
int		max_type_size(t_type *type);
char	*get_type_name(t_type *type);
bool	type_can_cast(t_type *from, t_type *to, bool is_explicit);
t_type	*type_array_to_ptr(t_type *type);
bool	can_use_arrow(t_type *type);
bool	can_use_dot(t_type *type);
bool	is_memory_type(t_type *type);

bool	find_enum(char *str, int len, t_defenum **res_def, int *res_value);
t_lvar	*find_lvar(t_deffunc *func, char *str, int len);
t_defvar	*find_global(char *str, int len);

void	parse(void);
void	analyze();
void	codegen(void);

#endif
