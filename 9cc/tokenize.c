#include "9cc.h"
#include "charutil.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

static t_token *new_token(t_tokenkind kind, t_token *last, char *str, int len)
{
	t_token *tok = calloc(1, sizeof(t_token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	tok->strlen_actual = len;
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
		if (!isalnum(*p) && !is_underscore(*p))
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

static int	match_operators(char **str, t_token **last)
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

static int	match_word(char **str, t_token **last, char *needle, t_tokenkind kind)
{
	int		len;

	len = strlen(needle);
	if (strncmp(*str, needle, len) != 0)
		return 0;
	if (isalnum((*str)[len]) || is_underscore((*str)[len]))
		return 0;

	*last = new_token(kind, *last, *str, len);
	*str += len;
	return len;
}

static bool	match_var_name(char **str, t_token **last)
{
	if (!can_use_beginning_of_var(**str))
		return (false);
	*last = new_token(TK_IDENT, *last, *str, read_var_name(*str));
	*str += (*last)->len;
	return (true);
}

static bool	match_number(char **str, t_token **last)
{
	if (!isdigit(**str))
		return (false);
	*last = new_token(TK_NUM, *last, *str, 1);
	(*last)->val = strtol(*str, str, 10);
	return (true);
}

static bool	match_strlit(char **str, t_token **last)
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
		else if (**str == '\0')
			error_at(*str, "文字列が終了しませんでした");
		(*last)->len++;
		(*last)->strlen_actual++;
		(*str)++;
	}
	if (**str != '"')
		error_at(*str, "文字列が終了しませんでした");
	(*str)++;
	return (true);
}

static bool	match_charlit(char **str, t_token **last)
{
	if (**str != '\'')
		return (false);

	(*str)++;
	*last = new_token(TK_CHAR_LITERAL, *last, *str, 1);

	if (**str == '\\')
	{
		*str += 1;
		(*last)->strlen_actual++;
		if (!is_escapedchar(**str))
			error_at(*str, "不明なエスケープシーケンスです (match_charlit)");
	}

	(*str)++;
	if (**str != '\'')
		error_at(*str, "文字が終了しませんでした : %c", **str);
	(*str)++;
	return (true);
}

t_token	*tokenize(char *p)
{
	t_token	head;
	t_token	*last;

	last = &head;
	while (*p)
	{
		if (skipspace(&p))									continue ;
		if (match_operators(&p, &last))						continue ;

		if (match_word(&p, &last, "return",	TK_RETURN))		continue ;
		if (match_word(&p, &last, "if",		TK_IF))			continue ;
		if (match_word(&p, &last, "else",	TK_ELSE))		continue ;
		if (match_word(&p, &last, "while",	TK_WHILE))		continue ;
		if (match_word(&p, &last, "for",	TK_FOR))		continue ;
		if (match_word(&p, &last, "do",		TK_DO))			continue ;
		if (match_word(&p, &last, "switch",	TK_SWITCH))		continue ;
		if (match_word(&p, &last, "sizeof",	TK_SIZEOF))		continue ;
		if (match_word(&p, &last, "struct",	TK_STRUCT))		continue ;
		if (match_word(&p, &last, "int",	TK_INT))		continue ;
		if (match_word(&p, &last, "char",	TK_CHAR))		continue ;
		if (match_word(&p, &last, "_Bool",	TK_BOOL))		continue ;
		if (match_word(&p, &last, "float",	TK_FLOAT))		continue ;
		if (match_word(&p, &last, "void",	TK_VOID))		continue ;
		if (match_word(&p, &last, "union",	TK_UNION))		continue ;
		if (match_word(&p, &last, "enum",	TK_ENUM))		continue ;
		if (match_word(&p, &last, "case",	TK_CASE))		continue ;
		if (match_word(&p, &last, "break",	TK_BREAK))		continue ;
		if (match_word(&p, &last, "continue",TK_CONTINUE))	continue ;
		if (match_word(&p, &last, "default",TK_DEFAULT))	continue ;
		if (match_word(&p, &last, "static",	TK_STATIC))		continue ;
		if (match_word(&p, &last, "typedef",TK_TYPEDEF))	continue ;
		if (match_word(&p, &last, "extern",	TK_EXTERN))		continue ;
		if (match_word(&p, &last, "inline",	TK_INLINE))		continue ;

		if (match_var_name(&p, &last))						continue ;
		if (match_number(&p, &last))						continue ;

		if (match_strlit(&p, &last))						continue ;
		if (match_charlit(&p, &last))						continue ;

		error_at(p, "failed to t_tokenize");
	}
	
	new_token(TK_EOF, last, p, 0);
	return head.next;
}
