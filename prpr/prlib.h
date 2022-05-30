#ifndef PRLIB_H
# define PRLIB_H

# include <stdbool.h>

// 識別子、予約語、単語
// 数字
// 文字列リテラル
// 文字リテラル
// 記号
// End Of Directive
// EOF
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

typedef struct s_tokenize_env
{
	char	*str;
	Token	*token;
	bool	can_define_dir;
}	TokenizeEnv;

typedef enum e_node
{
	ND_CODES,
	ND_INCLUDE,
	ND_DEFINE_NAME,
	ND_DEFINE_MACRO,
	ND_IFDEF,
	ND_IFNDEF
} NodeKind;

typedef struct s_node
{
	NodeKind		kind;
	Token			*codes;
	int				codes_len;
	char			*filename;
	bool			is_std_include;
	char			*name;
	struct s_node	*elif;
	struct s_node	*next;
}	Node;

typedef struct s_parse_env
{
	Token	*token;
	Node	*node;
}	ParseEnv;

char	*read_file(char *name);

int		start_with(char *haystack, char *needle);
char	*skip_space(char *p);
char	*read_line(char *p);

void	error(char *fmt, ...);
void	error_at(char *at, char *fmt, ...);

bool	is_ident_prefix(char str);
char	*read_ident(char *str);
char	*read_number(char *str);
int		is_reserved_word(char *str);

bool	is_number(char str);
bool	is_alnum(char str);
bool	is_symbol(char str);

Token	*tokenize(char *str);
Node	*parse(Token *tok);

#endif
