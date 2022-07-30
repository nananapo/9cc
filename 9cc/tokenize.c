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

// check str is start with needle
// if so, returns length of needle.
// otherwise returns 0.
int	match_word(char **str, Token **last, char *needle, TokenKind kind)
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

static int	match_operators(char *str)
{
	int	i;

	for(i=0;strlen(operators[i]) > 0;i++)
		if (strncmp(operators[i], str, strlen(operators[i])) == 0)
			return strlen(operators[i]);
	return 0;
}

// read var name
// return length
int	read_var_name(char *p)
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

Token	*tokenize(char *p)
{
	Token	head;
	Token	*cur;
	int		op_len;

	head.next = NULL;
	cur = &head;
	while (*p)
	{
		if (isspace(*p)) { p++; continue ; }

		op_len = match_operators(p);
		if (op_len)
		{
			cur = new_token(TK_RESERVED, cur, p, op_len);
			p += cur->len;
			continue ;
		}

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

		if (can_use_beginning_of_var(*p))
		{
			cur = new_token(TK_IDENT, cur, p, read_var_name(p));
			p += cur->len;
			continue ;
		}
		if (isdigit(*p))
		{
			cur = new_token(TK_NUM, cur, p, 1);
			cur->val = strtol(p, &p, 10);
			continue ;
		}
		if (*p == '"')
		{
			cur = new_token(TK_STR_LITERAL, cur, ++p, 0);
			cur->len = 0;
			cur->strlen_actual = 0;
			while (*p)
			{
				if (*p == '\\')
				{
					p++;
					if (!is_escapedchar(*p))
						error_at(p, "不明なエスケープシーケンスです");
					cur->len++;
				}
				else if (*p == '"')
					break;
				cur->len++;
				cur->strlen_actual++;
				p++;
			}
			if (*p != '"')
				error_at(p, "文字列が終了しませんでした");
			p++;
			continue ;
		}
		if (*p == '\'')
		{
			cur = new_token(TK_CHAR_LITERAL, cur, ++p, 1);
			cur->len = 1;
			cur->strlen_actual = 1;
			if (*p == '\\')
			{
				p++;
				cur->strlen_actual++;
				if (!is_escapedchar(*p))
					error_at(p, "不明なエスケープシーケンスです");
			}
			p++;
			if (*p != '\'')
				error_at(p, "文字が終了しませんでした : %c", *p);
			p++;
			continue ;
		}

		error_at(p, "failed to Tokenize");
	}
	
	new_token(TK_EOF, cur, p, 0);
	return head.next;
}
