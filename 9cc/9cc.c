#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Token Token;
typedef struct Node Node;

Node	*expr();

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
	ND_GREATER,
	ND_GREATEREQ,
} NodeKind;

struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

Token	*token;

char	*user_input;

void	error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void	error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

bool	consume(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len) != 0)
		return false;
	token = token->next;
	return true;
}

void	expect(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len) != 0)
		error_at(token->str, "'%c'ではありません", op);
	token = token->next;
}

int expect_number()
{
	int val;

	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
	val = token->val;
	token = token->next;
	return val;	
}

bool	at_eof()
{
	return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str)
{
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	tok->len = 1;
	return tok;
}

char *reserved_words[] = {
	">=", "<=", "==", "!=",
	">", "<", "=",
	"+", "-", "*", "/",
	"(", ")"
};

char	*match_reserved_word(char *str)
{
	int	i;

	i = -1;
	while (++i < 13)
		if (strncmp(reserved_words[i], str, strlen(reserved_words[i])) == 0)
			return reserved_words[i];
	return NULL;
}

Token	*tokenize(char *p)
{
	Token head;
	head.next = NULL;
	Token *cur = &head;
	char	*res_result;

	while (*p)
	{
		if (isspace(*p))
		{
			p++;
			continue;
		}
		
		res_result = match_reserved_word(p);
		if (res_result != NULL)
		{
			cur = new_token(TK_RESERVED, cur, p++);
			cur->len = strlen(res_result);
			continue;
		}
		if (isdigit(*p))
		{
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}
		error_at(p, "failed to Tokenize");
	}
	
	new_token(TK_EOF, cur, p);
	return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

Node *primary()
{
	if (consume("("))
	{
		Node *node = expr();
		expect(")");
		return node;
	}
	return new_node_num(expect_number());
}

Node *unary()
{
	if (consume("+"))
		return primary();
	if (consume("-"))
		return  new_node(ND_SUB, new_node_num(0), primary());
	return primary();
}

Node *mul()
{
	Node *node = unary();
	for (;;)
	{
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return node;
	}
}

Node *add()
{
	Node *node = mul();
	for (;;)
	{
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return node;
	}
}

Node *relational()
{
	Node *node = add();
	for (;;)
	{
		if (consume("<"))
			node = new_node(ND_LESS, node, add());
		else if (consume("<="))
			node = new_node(ND_LESSEQ, node, add());
		else if (consume(">"))
			node = new_node(ND_GREATER, node, add());
		else if (consume(">="))
			node = new_node(ND_GREATEREQ, node, add());
		else
			return node;
	}
}

Node *equality()
{
	Node *node = relational();
	for (;;)
	{
		if (consume("=="))
			node = new_node(ND_EQUAL, node, relational());
		else if (consume("!="))
			node = new_node(ND_NEQUAL, node, relational());
		else
			return node;
	}
}

Node *expr()
{
	return equality();
}

void	gen(Node *node)
{
	if (node->kind == ND_NUM)
	{
		printf("    push %d\n", node->val);
		return;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("    pop rdi\n");
	printf("    pop rax\n");

	switch (node->kind)
	{
		case ND_ADD:
			printf("    add rax, rdi\n");
			break;
		case ND_SUB:
			printf("    sub rax, rdi\n");
			break;
		case ND_MUL:
			printf("    imul rax, rdi\n");
			break;
		case ND_DIV:
			printf("    cqo\n");
			printf("    idiv rdi\n");
			break;
		default:
			error("不明なノード");
			break;
	}
	printf("    push rax\n");
}

int main(int argc, char **argv)
{
	char	*str;
	int		sign;
	Node	*node;

	if (argc != 2)
	{
		fprintf(stderr, "引数の個数が正しくありません");
		return (1);
	}

	user_input = argv[1];
	token = tokenize(argv[1]);
	node = expr();

	printf(".intel_syntax noprefix\n");
	printf(".global _main\n");
	printf("_main:\n");
	
	gen(node);

	printf("    pop rax\n");
	printf("    ret\n");
	return (0);
}
