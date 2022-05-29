#ifndef PRLIB_H
# define PRLIB_H

# include <stdbool.h>

typedef enum
{
	TK_IDENT,		// 識別子、単語
	TK_NUM,			// 数字
	TK_STR_LIT,		// 文字列リテラル
	TK_CHAR_LIT,	// 文字リテラル
	TK_RESERVED,	// 記号
	TK_DIRECTIVE,	// ディレクティブの宣言
	TK_DIR_WORD,	// その他のディレクティブ内のワード
	TK_EOF
} TokenKind;

typedef struct s_token
{
	TokenKind		kind;
	char			*str;
	int				len;
	struct s_token	*next;
}	Token;

typedef struct s_define
{
	char			*name;
	int				name_len;
	char			*replace;
	int				replace_len;
	struct s_define	*next;
}	t_define;

typedef struct s_parse_env
{
	char	*str;
	Token	*token;
	bool	can_define_dir;
}	ParseEnv;

void	include_file(char *filename, int len);

char	*read_file(char *name);
void	process(char *str);

int		start_with(char *haystack, char *needle);
char	*skip_space(char *p);
char	*strchr_line(char *p, char needle);
char	*read_token(char *p);
char	*read_line(char *p);

char	*read_include_directive(char *p);
char	*read_define_directive(char *p);
char	*read_ifdef_directive(char *p, bool is_ifdef);

void	error(char *fmt, ...);
void	error_at(char *at, char *fmt, ...);

Token	*parse(char *str);

#endif
