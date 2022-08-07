#ifndef PRLIB_H
# define PRLIB_H

# include <stdbool.h>

# define DEBUG

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
	ND_INIT,
	ND_CODES,
	ND_INCLUDE,
	ND_DEFINE_MACRO,
	ND_UNDEF,
	ND_IFDEF,
	ND_IFNDEF
} NodeKind;

typedef struct s_str_elem
{
	char				*str;
	struct s_str_elem	*next;
}	StrElem;

typedef struct s_node
{
	NodeKind		kind;

// codes用
	Token			*codes; // defineと共有
	int				codes_len;
// include用
	char			*file_name;
	bool			is_std_include;
// define用
	char			*macro_name; // ifと共有
	StrElem			*params;
	struct s_node	*stmt;
// if
	struct s_node	*elif;
	struct s_node	*els;

	struct s_node	*next;
}	Node;

typedef struct s_parse_env
{
	Token	*token;
	Node	*node;
}	ParseEnv;

typedef struct s_macro
{
	char			*name;

	StrElem			*params;
	Token			*codes;
	int				codes_len;

	struct s_macro	*next;
}	Macro;

typedef struct s_gen_env
{
	Macro	*macros;
	int		print_count;
	int		nest_count;
	char	*stddir;
}	GenEnv;

char	*my_strndup(char *s, int len);

void	debug(char *fmt, ...);

char	*read_file(char *name);
char	*getdir(char *full);

int		start_with(char *haystack, char *needle);
char	*skip_space(char *p);

void	error(char *fmt, ...);
void	error_at(char *at, char *fmt, ...);

char	*read_ident(char *str);
char	*read_number(char *str);
int		is_reserved_word(char *str);

bool	is_symbol(char str);
void	add_str_elem(StrElem **list, char *str);

Token	*tokenize(char *str);
Node	*parse(Token **tok, int nest);
void	gen(Node *node);

void	set_currentdir(char *stddir, char *filename);

Node	*create_node(NodeKind kind);

char	*strlit_to_str(char *str, int len);

#endif
