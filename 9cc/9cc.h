#ifndef NINECC_H
# define NINECC_H

# define ASM_MOV "mov"
# define ASM_PUSH "push"
# define RAX "rax"
# define RBP "rbp"
# define RSP "rsp"
#include <stdbool.h>

typedef struct Token Token;
typedef struct Node Node;
typedef struct LVar LVar;

typedef struct Type	Type;

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
	TK_EOF,
	TK_SIZEOF,
} TokenKind;

struct Token
{
	TokenKind	kind;
	Token		*next;
	int			val;
	char		*str;
	int			len;
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
	ND_DEFVAR
} NodeKind;

typedef enum
{
	INT,
	PTR,
	ARRAY
} PrimitiveType;

struct Type
{
	PrimitiveType	ty;
	Type		*ptr_to;

	int array_size;

	// arg用
	Type		*next;
};

struct Node
{
	NodeKind	kind;
	Node		*lhs;
	Node		*rhs;

	// num
	int			val;

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
};

struct	LVar
{
	LVar	*next;

	char	*name;
	int		len;
	int		offset;

	Type	*type;
};

//int	is_block_node(Node *node);

int	is_alnum(char c);
int	can_use_beginning_of_var(char c);

void	error(char *fmt, ...);
void	error_at(char *loc, char *fmt, ...);

bool	consume(char *op);
bool	consume_with_type(TokenKind kind);
Token	*consume_ident();
Token	*consume_ident_str(char *p);
void	expect(char *op);
int		expect_number();
bool	at_eof();

Token	*tokenize(char *p);
Token *new_token(TokenKind kind, Token *cur, char *str);

Node	*new_node(NodeKind kind, Node *lhs, Node *rhs);

// Type
Type	*new_primitive_type(PrimitiveType pri);
int		type_size(Type *type);
Type	*new_primitive_type(PrimitiveType pri);
Type	*new_type_ptr_to(Type *ptr_to);
Type	*new_type_array(Type *ptr_to);
bool	type_equal(Type *t1, Type *t2);

Type	*consume_type_before();
void	expect_type_after(Type **type);

void	gen(Node *node);

void	program();

#endif
