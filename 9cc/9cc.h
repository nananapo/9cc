#ifndef NINECC_H
# define NINECC_H

#include <stdbool.h>

typedef struct Token Token;
typedef struct Node Node;

typedef enum {
	TK_RESERVED,
	TK_NUM,
	TK_EOF,
} TokenKind;

struct Token {
	TokenKind	kind;
	Token		*next;
	int			val;
	char		*str;
	int			len;
};

typedef enum {
	ND_NUM,
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_EQUAL,
	ND_NEQUAL,
	ND_LESS,
	ND_LESSEQ,
} NodeKind;

struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

Node	*expr();
void	error_at(char *loc, char *fmt, ...);

bool	consume(char *op);
void	expect(char *op);
int		expect_number();
bool	at_eof();

Token	*tokenize(char *p);

void	gen(Node *node);

#endif
