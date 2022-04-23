#ifndef NINECC_H
# define NINECC_H

#include <stdbool.h>

typedef struct Token Token;
typedef struct Node Node;
typedef struct LVar LVar;

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
	ND_DUMMY,
	ND_FUNCDEF,
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

struct Node
{
	NodeKind	kind;
	Node		*lhs;
	Node		*rhs;
	int			val;
	int			offset;

	// else of if
	Node		*els;

	// for
	Node		*for_init;
	Node		*for_if;
	Node		*for_next;

	// call & func
	char		*fname;
	int			flen;
	Node		*args; // call only
	
	// func
	int			argdef_count;
	int			locals_len;
};

struct	LVar
{
	LVar	*next;
	char	*name;
	int		len;
	int		offset;
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
