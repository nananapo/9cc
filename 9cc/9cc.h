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
	ND_ELSE,
	ND_WHILE,
	ND_FOR
} NodeKind;

struct Node
{
	NodeKind	kind;
	Node		*lhs;
	Node		*rhs;
	int			val;
	int			offset;
};

struct	LVar
{
	LVar	*next;
	char	*name;
	int		len;
	int		offset;
};


int	is_alnum(char c);
int	can_use_beginning_of_var(char c);

Node	*expr();
void	error(char *fmt, ...);
void	error_at(char *loc, char *fmt, ...);

bool	consume(char *op);
bool	consume_with_type(TokenKind kind);
Token	*consume_ident();
void	expect(char *op);
int		expect_number();
bool	at_eof();

Token	*tokenize(char *p);

void	gen(Node *node);

void	program();
#endif
