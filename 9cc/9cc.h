#ifndef NINECC_H
# define NINECC_H

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
	PTR
} PrimitiveType;

struct Type
{
	PrimitiveType	ty;
	Type		*ptr_to;

	// arg用
	Type		*next;
};

struct Node
{
	NodeKind	kind;
	Node		*lhs;
	Node		*rhs;
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

int	is_block_node(Node *node);

int	is_alnum(char c);
int	can_use_beginning_of_var(char c);

Node	*expr();
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

void	gen(Node *node);

void	program();
#endif
