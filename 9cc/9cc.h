#ifndef NINECC_H
# define NINECC_H

#include <stdbool.h>

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
	int				val;
	char			*str;
	int				len;
	int				strlen_actual;
} Token;

typedef enum NodeKind
{
	ND_FUNCDEF,
	ND_PROTOTYPE,
	ND_TYPEDEF,
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

	ND_LVAR,

	ND_SHIFT_LEFT,
	ND_SHIFT_RIGHT,

	ND_BITWISE_OR,
	ND_BITWISE_NOT,
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

	ND_DEFVAR,
	ND_DEFVAR_GLOBAL,
 	ND_LVAR_GLOBAL,
	ND_STR_LITERAL,

	ND_STRUCT_DEF,
	ND_UNION_DEF,
	ND_ENUM_DEF,

	ND_MEMBER_VALUE, // .
	ND_MEMBER_PTR_VALUE, // ->

	ND_CAST,
	ND_PARENTHESES,
	ND_BREAK,
	ND_CONTINUE,
	ND_DEFAULT,
} NodeKind;

typedef enum PrimitiveType
{
	INT,
	CHAR,
	PTR,
	ARRAY,
	STRUCT,
	ENUM,
	UNION,
	BOOL,
	VOID
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

	// arg用
	struct Type		*next;

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

SBData	*sbdata_new(bool isswitch, int start, int end);
void	sb_forwhile_start(int startlabel, int endlabel);
void	sb_switch_start(Type *type, int endlabel, int defaultLabel);
SBData	*sb_end(void);
SBData	*sb_peek(void);
SBData	*sb_search(bool	isswitch);

typedef struct	LVar
{
	struct LVar	*next;

	char		*name;
	int			len;
	int			offset;

	bool		is_arg;
	int			arg_regindex;

	Type		*type;
} LVar;

typedef struct Node
{
	NodeKind	kind;
	struct Node	*lhs;
	struct Node	*rhs;

	char		*var_name;
	int			var_name_len;
	Type		*type;

	bool		is_static;

	// global var
	bool		is_extern;
	struct Node	*global_assign; // 初期化用

	// local var
	LVar		*lvar;

	// num
	int			val;

	// str
	int			str_index;

	// else of if
	struct Node	*elsif;
	struct Node	*els;

	// for
	struct Node	*for_init;
	struct Node	*for_if;
	struct Node	*for_next;

	// call & func
	char		*fname;
	int			flen;
	int			argdef_count;
	
	// call
	struct Node	*args;
	
	// func
	Type		*arg_type;
	Type		*ret_type;
	LVar 		*locals;
	bool		is_variable_argument;

	// general
	struct Node	*next;

	MemberElem	*elem;
	bool		is_struct_address;

	// valとlabelでswicth-case
	int			switch_label;
	SwitchCase	*switch_cases;
	bool		switch_has_default;
}	Node;

typedef struct s_str_literal_elem
{
	char						*str;
	int							len;
	struct s_str_literal_elem	*next;
	int							index;
}	t_str_elem;

typedef struct	s_find_enum_res
{
	Type	*type;
	int		value;
}	FindEnumRes;

#include "list.h"

typedef struct s_parseresult
{
	Token			*token;
	Node			*code[1000];
	Node			*func_defs[1000];
	Node			*func_protos[1000];
	Node			*global_vars[1000];
	t_str_elem		*str_literals;
	StructDef		*struct_defs[1000];
	EnumDef			*enum_defs[1000];
	UnionDef		*union_defs[1000];
	LVar			*locals;


	t_linked_list	*type_alias;
}	ParseResult;

typedef struct s_typedefpair
{
	char	*name;
	int		name_len;
	Type	*type;
}	TypedefPair;

char	*read_file(char	*name);

int	align_to(int a, int to);

int	can_use_beginning_of_var(char c);
int	is_escapedchar(char c);
int get_char_to_int(char *p, int len);

void	error(char *fmt, ...);
void	error_at(char *loc, char *fmt, ...);

Token	*tokenize(char *p);

Node	*new_node(NodeKind kind, Node *lhs, Node *rhs);
Node	*new_node_num(int val);


// Type
Type	*new_primitive_type(PrimitiveType pri);
int		type_size(Type *type);
Type	*new_primitive_type(PrimitiveType pri);
Type	*new_type_ptr_to(Type *ptr_to);
Type	*new_type_array(Type *ptr_to);
Type	*new_struct_type(ParseResult *env, char *name, int len);
Type	*new_enum_type(ParseResult *env, char *name, int len);
Type	*new_union_type(ParseResult *env, char *name, int len);
bool	is_integer_type(Type *type);
bool	is_pointer_type(Type *type);
bool	is_declarable_type(Type *type);
bool	can_compared(Type *l, Type *r, Type **lt, Type **rt);
bool	type_equal(Type *t1, Type *t2);
char	*type_regname(Type *type);
MemberElem	*get_member_by_name(Type *type, char *name, int len);
int		max_type_size(Type *type);
char	*get_type_name(Type *type);
bool	type_can_cast(Type *from, Type *to, bool is_explicit);
Type	*type_array_to_ptr(Type *type);
bool	can_use_arrow(Type *type);
bool	can_use_dot(Type *type);

char	*get_str_literal_name(int index);

void	gen(Node *node);

ParseResult	*parse(Token *tok);

#endif
