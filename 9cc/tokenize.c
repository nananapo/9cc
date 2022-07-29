#include "9cc.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

extern char		*user_input;

static char *reserved_words[42] = {
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

static Token *new_token(TokenKind kind, Token *cur, char *str)
{
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	tok->len = 1;
	return tok;
}

static char	*match_reserved_word(char *str)
{
	int	i;

	for(i=0;strlen(reserved_words[i]) > 0;i++)
		if (strncmp(reserved_words[i], str, strlen(reserved_words[i])) == 0)
			return reserved_words[i];
	return NULL;
}

static bool	issymbol(char c)
{
	return (c == '_');
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

// check str is start with needle
// if so, returns length of needle.
// otherwise returns 0.
int	match_word(char *str, char *needle)
{
	int	len = strlen(needle);

	if (strncmp(str, needle, len) != 0)
		return 0;
	if (isalnum(str[len]) || issymbol(str[len]))
		return 0;
	return len;
}

Token	*tokenize(char *p)
{
	Token	head;
	Token	*cur;
	char	*res_result;

	head.next = NULL;
	cur = &head;
	while (*p)
	{
		if (isspace(*p))
		{
			p++;
			continue ;
		}

		res_result = match_reserved_word(p);
		if (res_result != NULL)
		{
			cur = new_token(TK_RESERVED, cur, p);
			cur->len = strlen(res_result);
			p += cur->len;
			continue ;
		}

		if (match_word(p, "return"))
		{
			cur = new_token(TK_RETURN, cur, p);
			cur->len = 6;
			p += 6;
			continue ;
		}
		if (match_word(p, "if"))
		{
			cur = new_token(TK_IF, cur, p);
			cur->len = 2;
			p += 2;
			continue ;
		}
		if (match_word(p, "else"))
		{
			cur = new_token(TK_ELSE, cur, p);
			cur->len = 4;
			p += 4;
			continue ;
		}
		if (match_word(p, "while"))
		{
			cur = new_token(TK_WHILE, cur, p);
			cur->len = 5;
			p += 5;
			continue ;
		}
		if (match_word(p, "for"))
		{
			cur = new_token(TK_FOR, cur, p);
			cur->len = 3;
			p += 3;
			continue ;
		}
		if (match_word(p, "do"))
		{
			cur = new_token(TK_DO, cur, p);
			cur->len = 2;
			p += 2;
			continue ;
		}
		if (match_word(p, "sizeof"))
		{
			cur = new_token(TK_SIZEOF, cur, p);
			cur->len = 6;
			p += 6;
			continue ;
		}
		if (match_word(p, "struct"))
		{
			cur = new_token(TK_STRUCT, cur, p);
			cur->len = 6;
			p += 6;
			continue ;
		}
		if (match_word(p, "union"))
		{
			cur = new_token(TK_UNION, cur, p);
			cur->len = 5;
			p += 5;
			continue ;
		}
		if (match_word(p, "break"))
		{
			cur = new_token(TK_BREAK, cur, p);
			cur->len = 5;
			p += 5;
			continue ;
		}
		if (match_word(p, "continue"))
		{
			cur = new_token(TK_CONTINUE, cur, p);
			cur->len = 8;
			p += 8;
			continue ;
		}
		if (match_word(p, "switch"))
		{
			cur = new_token(TK_SWITCH, cur, p);
			cur->len = 6;
			p += 6;
			continue ;
		}
		if (match_word(p, "case"))
		{
			cur = new_token(TK_CASE, cur, p);
			cur->len = 4;
			p += 4;
			continue ;
		}
		if (match_word(p, "default"))
		{
			cur = new_token(TK_DEFAULT, cur, p);
			cur->len = 7;
			p += 7;
			continue ;
		}
		if (match_word(p, "static"))
		{
			cur = new_token(TK_STATIC, cur, p);
			cur->len = 6;
			p += 6;
			continue ;
		}
		if (match_word(p, "typedef"))
		{
			cur = new_token(TK_TYPEDEF, cur, p);
			cur->len = 7;
			p += 7;
			continue ;
		}
		if (match_word(p, "enum"))
		{
			cur = new_token(TK_ENUM, cur, p);
			cur->len = 4;
			p += 4;
			continue ;
		}
		if (match_word(p, "extern"))
		{
			cur = new_token(TK_EXTERN, cur, p);
			cur->len = 6;
			p += 6;
			continue ;
		}
		if (match_word(p, "inline"))
		{
			cur = new_token(TK_INLINE, cur, p);
			cur->len = 6;
			p += 6;
			continue ;
		}
		if (can_use_beginning_of_var(*p))
		{
			cur = new_token(TK_IDENT, cur, p);
			cur->len = read_var_name(p);
			p += cur->len;
			continue ;
		}
		if (isdigit(*p))
		{
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue ;
		}
		if (*p == '"')
		{
			cur = new_token(TK_STR_LITERAL, cur, ++p);
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
			cur = new_token(TK_CHAR_LITERAL, cur, ++p);
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
	
	new_token(TK_EOF, cur, p);
	return head.next;
}
