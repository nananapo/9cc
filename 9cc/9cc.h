#ifndef NINECC_H
# define NINECC_H

#include <stdbool.h>

# define ASM_MOV "mov"
# define ASM_PUSH "push"
# define ASM_LEA "lea"

# define RAX "rax"
# define RDI "rdi"
# define RBP "rbp"
# define RSP "rsp"

# define EAX "eax"
# define EDI "edi"

# define AL "al"
# define DIL "dil"

# define BYTE_PTR "byte ptr"
# define WORD_PTR "word ptr"
# define DWORD_PTR "dword ptr"

typedef struct Token Token;
typedef struct Node Node;
typedef struct LVar LVar;

typedef struct Type	Type;
typedef struct s_struct_definition StructDef;
typedef struct s_struct_members StructMemberElem;

typedef enum
{
	TK_RESERVED,
	TK_IDENT,
	TK_NUM,
	TK_RETURN,
	TK_IF,
	TK_ELSE,
	TK_WHILE,
	TK_FOR,
	TK_STR_LITERAL,
	TK_EOF,
	TK_SIZEOF,
	TK_STRUCT
} TokenKind;

struct Token
{
	TokenKind	kind;
	Token		*next;
	int			val;
	char		*str;
	int			len;
	int			strlen_actual;
};

typedef enum
{
	ND_FUNCDEF,
	ND_PROTOTYPE,
	ND_CALL,
	ND_BLOCK,
	ND_NUM,
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_EQUAL,
	ND_NEQUAL,
	ND_LESS,
	ND_LESSEQ,
	ND_ASSIGN,
	ND_LVAR,
	ND_RETURN,
	ND_IF,
	ND_WHILE,
	ND_FOR,
	ND_ADDR,
	ND_DEREF,
	ND_DEFVAR,
	ND_GLOBAL,
 	ND_LVAR_GLOBAL,
	ND_STR_LITERAL,
	ND_STRUCT_DEF,
	ND_STRUCT_VALUE,
	ND_STRUCT_PTR_VALUE
} NodeKind;

typedef enum
{
	INT,
	CHAR,
	PTR,
	ARRAY,
	STRUCT
} PrimitiveType;

struct Type
{
	PrimitiveType	ty;
	Type		*ptr_to;

	int array_size;

	// arg用
	Type		*next;

	StructDef	*strct;
};

struct Node
{
	NodeKind	kind;
	Node		*lhs;
	Node		*rhs;

	char		*var_name;
	int			var_name_len;

	// num
	int			val;

	// str
	int			str_index;

	// ident
	int			offset;
	Type		*type;

	// else of if
	Node		*els;

	// for
	Node		*for_init;
	Node		*for_if;
	Node		*for_next;

	// call & func
	char		*fname;
	int			flen;
	int			argdef_count;
	
	// call
	Node		*args;
	
	// func
	int			locals_len;
	Type		*arg_type;
	Type		*ret_type;
	int			stack_size;
	LVar 		*locals;

	// general
	Node		*next;

	StructMemberElem	*struct_elem;
};

struct	LVar
{
	LVar	*next;

	char	*name;
	int		len;
	int		offset;

	Type	*type;
};

typedef struct s_str_literal_elem
{
	char						*str;
	int							len;
	struct s_str_literal_elem	*next;
	int							index;
}	t_str_elem;

struct s_struct_members
{
	Type					*type;
	char					*name;
	int						name_len;
	int						offset;
	struct s_struct_members	*next;
};

struct s_struct_definition
{
	char				*name;
	int					name_len;
	int					mem_size; // -1ならまだ定義されていない
	StructMemberElem	*members;
};

int	align_to(int a, int to);

int	is_alnum(char c);
int	can_use_beginning_of_var(char c);

void	error(char *fmt, ...);
void	error_at(char *loc, char *fmt, ...);

bool	consume(char *op);
bool	consume_with_type(TokenKind kind);
Token	*consume_ident();
Token	*consume_ident_str(char *p);
Token	*consume_str_literal();
void	expect(char *op);
int		expect_number();
bool	 consume_number(int *result);
bool	at_eof();

Type	*consume_type_before();
void	expect_type_after(Type **type);
void	consume_type_ptr(Type **type);

Token	*tokenize(char *p);
Token 	*new_token(TokenKind kind, Token *cur, char *str);

Node	*new_node(NodeKind kind, Node *lhs, Node *rhs);

Node	*find_global(char *str, int len);

// Type
Type	*new_primitive_type(PrimitiveType pri);
int		type_size(Type *type);
Type	*new_primitive_type(PrimitiveType pri);
Type	*new_type_ptr_to(Type *ptr_to);
Type	*new_type_array(Type *ptr_to);
Type	*new_struct_type(char *name, int len);
bool	is_integer_type(Type *type);
bool	is_pointer_type(Type *type);
bool	can_assign(Type *l, Type *r);
bool	can_compared(Type *l, Type *r);
bool	type_equal(Type *t1, Type *t2);
char	*type_regname(Type *type);
StructMemberElem	*struct_get_member(StructDef *strct, char *name, int len);

char	*get_str_literal_name(int index);

void	gen(Node *node);

void	program();

#endif
