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

typedef enum TokenKind
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

	TK_STRUCT,
	TK_ENUM,
	TK_UNION,

	TK_STATIC,
	TK_TYPEDEF,
	TK_EXTERN,
 	TK_INLINE
} TokenKind;

typedef struct Token
{
	TokenKind		kind;
	struct Token	*next;

	int				val;			// number
	char			*str;			// token str
	int				len;			// length
	int				strlen_actual;	// charlit
} Token;

typedef enum NodeKind
{
	ND_NONE,
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
	ND_SUB_UNARY
} NodeKind;

typedef enum PrimitiveType
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
} PrimitiveType;

typedef struct s_enum_definition
{
	char	*name;
	int		name_len;

	char	*kinds[1000];
	int		kind_len;
}	EnumDef;

typedef struct Type
{
	PrimitiveType	ty;
	struct Type		*ptr_to;

	int 			array_size;

	struct s_struct_definition
	{
		char				*name;
		int					name_len;
		int					mem_size; // -1ならまだ定義されていない
		struct s_members
		{
			struct Type			*type;
			char				*name;
			int					name_len;
			int					offset;
			struct s_members	*next;
		} *members;
	}	*strct;

	EnumDef			*enm;

	struct s_union_definition
	{
		char				*name;
		int					name_len;
		int					mem_size;
		struct s_members	*members;
	}	*unon;
} Type;

typedef struct s_struct_definition	StructDef;
typedef struct s_members			MemberElem;
typedef struct s_union_definition	UnionDef;

typedef struct	s_switchcase
{
	int					value;
	int					label;
	struct s_switchcase	*next;
}	SwitchCase;

typedef struct s_sbdata
{
	bool		isswitch;

	int			startlabel;
	int			endlabel;

	Type		*type;
	SwitchCase	*cases;

	int			defaultLabel;
}	SBData;

typedef struct s_lvar
{
	struct s_lvar	*next;

	char		*name;
	int			name_len;
	int			offset;

	bool		is_arg;
	int			arg_regindex;
	bool		is_dummy;

	Type		*type;
} LVar;

typedef struct s_str_literal_elem
{
	char	*str;
	int		len;
	int		index;
}	t_str_elem;

typedef struct Node
{
	NodeKind	kind;
	struct Node	*lhs;
	struct Node	*rhs;

	Type		*type;

	LVar		*lvar; // local var
	struct Node	*lvar_assign; // type ident = expr;

	t_str_elem	*def_str;
struct s_defvar
{
	char		*name;
	int			name_len;

	Type		*type;

	// for global variable
	bool		is_extern;
	bool		is_static;
	struct Node	*assign;
}	*var_global;

	struct Node	*global_assign_next;

	// num
	int			val;

	// else of if
	struct Node	*elsif;
	struct Node	*els;

	struct Node	*for_expr[3]; // for (0; 1; 2)

	// call
struct s_deffunc
{
	char	*name;
	int		name_len;

	Type	*type_return;

	int		argcount;

	char	*argument_names[20];
	int		argument_name_lens[20];
	Type	*type_arguments[20];

	bool	is_static;
	bool	is_prototype;
	bool	is_zero_argument;		// func(void)
	bool	is_variable_argument;	// func(, ...)

	LVar	*locals;
	struct Node	*stmt;
}	*funcdef;
	int		funccall_argcount;
	struct Node	*funccall_args[20];
	LVar	*funccall_argdefs[20];

	// 返り値がMEMORYかstructな関数を呼んだ時に結果を入れる場所
	LVar		*call_mem_stack;

	MemberElem	*elem;

	// valとlabelでswicth-case
	SwitchCase	*switch_cases;
	SwitchCase	*case_label;
	bool		switch_has_default;

	SBData		*block_sbdata;



	// analyze
	bool		is_analyzed;
	char		*analyze_source;

	char		*analyze_var_name;
	int			analyze_var_name_len;

	char		*analyze_funccall_name;
	int			analyze_funccall_name_len;

	char		*analyze_member_name;
	int			analyze_member_name_len;
}	Node;

typedef struct s_deffunc t_deffunc;
typedef struct s_defvar	t_defvar;

typedef struct s_typedefpair
{
	char	*name;
	int		name_len;
	Type	*type;
}	TypedefPair;


void	debug(char *fmt, ...);

char	*read_file(char	*name);

int	align_to(int a, int to);

void	error(char *fmt, ...);
void	error_at(char *loc, char *fmt, ...);

Token	*tokenize(char *p);

Node	*new_node(NodeKind kind, Node *lhs, Node *rhs);
Node	*new_node_num(int val);

// Type
Type	*new_primitive_type(PrimitiveType pri);
int		get_type_size(Type *type);
Type	*new_type_ptr_to(Type *ptr_to);
Type	*new_type_array(Type *ptr_to);
Type	*new_struct_type(char *name, int len);
Type	*new_enum_type(char *name, int len);
Type	*new_union_type(char *name, int len);
bool	is_integer_type(Type *type);
bool	is_pointer_type(Type *type);
bool	is_declarable_type(Type *type);
bool	can_compared(Type *l, Type *r, Type **lt, Type **rt);
bool	type_equal(Type *t1, Type *t2);
MemberElem	*get_member_by_name(Type *type, char *name, int len);
int		max_type_size(Type *type);
char	*get_type_name(Type *type);
bool	type_can_cast(Type *from, Type *to, bool is_explicit);
Type	*type_array_to_ptr(Type *type);
bool	can_use_arrow(Type *type);
bool	can_use_dot(Type *type);
bool	is_memory_type(Type *type);

bool	find_enum(char *str, int len, EnumDef **res_def, int *res_value);
LVar	*find_lvar(t_deffunc *func, char *str, int len);
t_defvar	*find_global(char *str, int len);

void	parse(void);
void	analyze();
void	codegen(void);

#endif
