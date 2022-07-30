#include "9cc.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>


bool	issymbol(char c);

extern char		*user_input;

static char *operators[42] = {
	"...","++", "--", "->", ".",
	">=", "<=", "==", "!=","||",
	"&&","+=", "-=", "*=", "/=",

	">>", "<<", 

	"%=",">", "<", "=","+", "^",
	"-","*", "/", "%", "&", "?", "|", "~",
	"!","(",")", "{", "}", 
	"[", "]",",", ";", ":",
	""
};

static Token *new_token(TokenKind kind, Token *last, char *str, int len)
{
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	last->next = tok;
	return tok;
}

// read var name
// return length
static int	read_var_name(char *p)
{
	int	l = 0;

	while (*p)
	{
		if (!isalnum(*p) && !issymbol(*p))
			return l;
		l++;
		p++;
	}
	return l;
}

static bool	skipspace(char **str)
{
	if (isspace(**str))
	{
		*str += 1;
		return (true);
	}
	return (false);
}

static int	match_operators(char **str, Token **last)
{
	int	i;
	int	len;

	for(i=0;(len = strlen(operators[i])) > 0;i++)
		if (strncmp(operators[i], *str, len) == 0)
		{
			*last = new_token(TK_RESERVED, *last, *str, len);
			*str += len;
			return len;
		}
	return 0;
}


// check str is start with needle
// if so, returns length of needle.
// otherwise returns 0.
static int	match_word(char **str, Token **last, char *needle, TokenKind kind)
{
	int		len;

	len = strlen(needle);
	if (strncmp(*str, needle, len) != 0)
		return 0;
	if (isalnum((*str)[len]) || issymbol((*str)[len]))
		return 0;

	*last = new_token(kind, *last, *str, len);
	*str += len;

	return len;
}

static bool	match_var_name(char **str, Token **last)
{
	if (!can_use_beginning_of_var(**str))
		return (false);
	*last = new_token(TK_IDENT, *last, *str, read_var_name(*str));
	*str += (*last)->len;
	return (true);
}

static bool	match_number(char **str, Token **last)
{
	if (!isdigit(**str))
		return (false);
	*last = new_token(TK_NUM, *last, *str, 1);
	(*last)->val = strtol(*str, str, 10);
	return (true);
}

static bool	match_strlit(char **str, Token **last)
{
	if (**str != '"')
		return (false);
	*last = new_token(TK_STR_LITERAL, *last, ++(*str), 0);
	(*last)->len = 0;
	(*last)->strlen_actual = 0;
	while (*str)
	{
		if (**str == '\\')
		{
			(*str)++;
			if (!is_escapedchar(**str))
				error_at(*str, "不明なエスケープシーケンスです");
			(*last)->len++;
		}
		else if (**str == '"')
			break;
		(*last)->len++;
		(*last)->strlen_actual++;
		(*str)++;
	}
	if (**str != '"')
		error_at(*str, "文字列が終了しませんでした");
	(*str)++;
	return (true);
}

static bool	match_charlit(char **str, Token **last)
{
	if (**str != '\'')
		return (false);
	*last = new_token(TK_CHAR_LITERAL, *last, ++(*str), 1);
	(*last)->len = 1;
	(*last)->strlen_actual = 1;
	if (**str == '\\')
	{
		(*str)++;
		(*last)->strlen_actual++;
		if (!is_escapedchar(**str))
			error_at(*str, "不明なエスケープシーケンスです");
	}
	(*str)++;
	if (**str != '\'')
		error_at(*str, "文字が終了しませんでした : %c", **str);
	(*str)++;
	return (true);
}

Token	*tokenize(char *p)
{
	Token	head;
	Token	*cur;

	head.next = NULL;
	cur = &head;
	while (*p)
	{
		if (skipspace(&p))									continue ;
		if (match_operators(&p, &cur))						continue ;

		if (match_word(&p, &cur, "return",	TK_RETURN))		continue ;
		if (match_word(&p, &cur, "if",		TK_IF))			continue ;
		if (match_word(&p, &cur, "else",	TK_ELSE))		continue ;
		if (match_word(&p, &cur, "while",	TK_WHILE))		continue ;
		if (match_word(&p, &cur, "for",		TK_FOR))		continue ;
		if (match_word(&p, &cur, "do",		TK_DO))			continue ;
		if (match_word(&p, &cur, "switch",	TK_SWITCH))		continue ;
		if (match_word(&p, &cur, "sizeof",	TK_SIZEOF))		continue ;
		if (match_word(&p, &cur, "struct",	TK_STRUCT))		continue ;
		if (match_word(&p, &cur, "union",	TK_UNION))		continue ;
		if (match_word(&p, &cur, "enum",	TK_ENUM))		continue ;
		if (match_word(&p, &cur, "case",	TK_CASE))		continue ;
		if (match_word(&p, &cur, "break",	TK_BREAK))		continue ;
		if (match_word(&p, &cur, "continue",TK_CONTINUE))	continue ;
		if (match_word(&p, &cur, "default",	TK_DEFAULT))	continue ;
		if (match_word(&p, &cur, "static",	TK_STATIC))		continue ;
		if (match_word(&p, &cur, "typedef",	TK_TYPEDEF))	continue ;
		if (match_word(&p, &cur, "extern",	TK_EXTERN))		continue ;
		if (match_word(&p, &cur, "inline",	TK_INLINE))		continue ;

		if (match_var_name(&p, &cur))						continue ;
		if (match_number(&p, &cur))							continue ;

		if (match_strlit(&p, &cur))							continue ;
		if (match_charlit(&p, &cur))						continue ;

		error_at(p, "failed to Tokenize");
	}
	
	new_token(TK_EOF, cur, p, 0);
	return head.next;
}
