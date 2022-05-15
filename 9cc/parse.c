#include "9cc.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

extern Token	*token;

extern char		*user_input;

extern LVar		*locals;

static char *reserved_words[] = {
	">=", "<=", "==", "!=",
	">", "<", "=",
	"+", "-", "*", "/", "&",
	"(", ")", "{", "}", "[", "]",
	",", ";",
	NULL
};

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

Token *new_token(TokenKind kind, Token *cur, char *str)
{
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	tok->len = 1;
	return tok;
}

char	*match_reserved_word(char *str)
{
	for(int i=0;reserved_words[i];i++)
		if (strncmp(reserved_words[i], str, strlen(reserved_words[i])) == 0)
			return reserved_words[i];
	return NULL;
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

bool consume_number(int *result)
{
	if (token->kind != TK_NUM)
		return false;
	*result = token->val;
	token = token->next;
	return true;	
}

bool	consume_with_type(TokenKind kind)
{
	if (token->kind != kind)
		return false;
	token = token->next;
	return true;
}

Token	*consume_ident()
{
	Token	*ret;
	if (token->kind != TK_IDENT)
		return NULL;
	ret = token;
	token = token->next;
	return ret;
}

Token	*consume_ident_str(char *p)
{
	Token	*ret;
	if (token->kind != TK_IDENT)
		return NULL;
	if (strncmp(p, token->str, strlen(p)) != 0)
		return NULL;
	ret = token;
	token = token->next;
	return ret;
}

Token	*consume_str_literal()
{
	Token	*ret;
	if (token->kind != TK_STR_LITERAL)
		return NULL;
	ret = token;
	token = token->next;
	return ret;
}

// 型宣言の前部分 (type ident arrayのtype部分)を読む
Type	*consume_type_before()
{
	Type	*type;
	Type	*tmp;

	// type name
	if (consume_ident_str("int"))
		type = new_primitive_type(INT);
	else if (consume_ident_str("char"))
		type = new_primitive_type(CHAR);
	else
		return (NULL);

	while (consume("*"))
	{
		tmp = new_primitive_type(PTR);
		tmp->ptr_to = type;
		type = tmp;
	}
	return type;
}

// 型宣言のarray部分を読む
void	expect_type_after(Type **type)// expect size
{
	int		size;

	while (consume("["))
	{
		*type = new_type_array(*type);
		if (!consume_number(&size))
			error_at(token->str, "配列のサイズが定義されていません");
		(*type)->array_size = size;
		if (!consume("]"))
			error_at(token->str, "]がありません");
	}
	return;
}

void	expect(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len) != 0)
		error_at(token->str, "'%s'ではありません", op);
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

// read var name
// return length
int	read_var_name(char *p)
{
	int	l = 0;

	while (*p)
	{
		if (!is_alnum(*p))
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
	if (is_alnum(str[len]))
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
			continue;
		}
		res_result = match_reserved_word(p);
		if (res_result != NULL)
		{
			cur = new_token(TK_RESERVED, cur, p);
			cur->len = strlen(res_result);
			p += cur->len;
			continue;
		}
		if (match_word(p, "return"))
		{
			cur = new_token(TK_RETURN, cur, p);
			cur->len = 6;
			p += 6;
			continue;
		}
		if (match_word(p, "if"))
		{
			cur = new_token(TK_IF, cur, p);
			cur->len = 2;
			p += 2;
			continue;
		}
		if (match_word(p, "else"))
		{
			cur = new_token(TK_ELSE, cur, p);
			cur->len = 4;
			p += 4;
			continue;
		}
		if (match_word(p, "while"))
		{
			cur = new_token(TK_WHILE, cur, p);
			cur->len = 5;
			p += 5;
			continue;
		}
		if (match_word(p, "for"))
		{
			cur = new_token(TK_FOR, cur, p);
			cur->len = 3;
			p += 3;
			continue;
		}
		if (match_word(p, "sizeof"))
		{
			cur = new_token(TK_SIZEOF, cur, p);
			cur->len = 6;
			p += 6;
			continue;
		}
		if (can_use_beginning_of_var(*p))
		{
			cur = new_token(TK_IDENT, cur, p);
			cur->len = read_var_name(p);
			p += cur->len;
			continue;
		}
		if (isdigit(*p))
		{
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
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
					switch (*p)
					{
						case '"':
						case 'a':
						case 'b':
						case 'f':
						case 'n':
						case 'r':
						case 'v':
						case '0':
							break;
						default:
							error_at(p, "不明なエスケープシーケンスです");
					}
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

		error_at(p, "failed to Tokenize");
	}
	
	new_token(TK_EOF, cur, p);
	return head.next;
}
