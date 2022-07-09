#include <stdio.h>
#include <stdbool.h>

typedef enum e_token_kind
{
	TK_IDENT,
	TK_NUM,
	TK_STR_LIT,
	TK_CHAR_LIT,
	TK_RESERVED,
	TK_EOD,
	TK_EOF
}	TokenKind;

typedef struct s_token
{
	TokenKind		kind;
	char			*str;
	int				len;
	bool			is_directive;
	bool			is_dq;
	struct s_token	*next;
}	Token;

int	main(void)
{
	Token	token;

	printf("sizeof Token %d\n", sizeof(Token));
	printf("kind	: %d\n", (void *)&token - (void *)&token.kind);
	printf("str	: %d\n", (void *)&token - (void *)&token.str);
	printf("len	: %d\n", (void *)&token - (void *)&token.len);
	printf("is_d	: %d\n", (void *)&token - (void *)&token.is_directive);
	printf("isdq	: %d\n", (void *)&token - (void *)&token.is_dq);
	printf("next	: %d\n", (void *)&token - (void *)&token.next);

	token.kind = 0;
	token.str = 0;
	token.len = 100000;
	token.is_directive = true;
	token.is_dq = true;
	token.next = 0;

	printf("%d\n", token.kind);
	printf("%d\n", token.str);
	printf("%d\n", token.len);
	printf("%d\n", token.is_directive);
	printf("%d\n", token.is_dq);
	printf("%d\n", token.next);
}
