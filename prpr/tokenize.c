#include "prlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

static Token	*get_last_token(TokenizeEnv *env)
{
	Token	*tok;

	tok = env->token;
	if (tok == NULL)
		return (tok);
	while (tok->next != NULL)
		tok = tok->next;
	return (tok);
}

Token	*add_token(TokenizeEnv *env, TokenKind kind, char *str, int len)
{
	Token	*tok;

	tok = (Token *)malloc(sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	tok->is_directive = false;
	tok->is_dq = false;
	tok->next = NULL;
	if (env->token == NULL)
		env->token = tok;
	else
		get_last_token(env)->next = tok;
	return (tok);
}

static bool	tokenize_comment(TokenizeEnv *env, int len)
{
	char	*tmp;

	if (len == 2 && strncmp(env->str, "//", 2) == 0)
	{
		env->str = strchr(env->str, '\n');
		return (true);
	}
	if (len == 2 && strncmp(env->str, "/*", 2) == 0)
	{
		tmp = env->str;
		env->str = strstr(env->str, "*/") + 2;
		if (env->str == NULL)
			error_at(tmp, "/*に対応する*/が見つかりませんでした");
		return (true);
	}
	if (len == 2 && strncmp(env->str, "*/", 2) == 0)
	{
		error_at(env->str, "*/に対応する/*が見つかりませんでした");
	}
	return (false);
}

static bool	tokenize_reserved_word(TokenizeEnv *env)
{
	int	len;

	len = is_reserved_word(env->str);
	if (len == 0)
		return (false);
	if (tokenize_comment(env, len))
		return (true);
	add_token(env, TK_RESERVED, env->str, len);
	env->str += len;
	return (true);
}

static bool	tokenize_new_line(TokenizeEnv *env)
{
	if (*env->str != '\n')
		return (false);
	env->str += 1;
	env->can_define_dir = true;
	return (true);
}

static bool	tokenize_ident(TokenizeEnv *env)
{
	char	*tmp;

	tmp = env->str;
	env->str = read_ident(env->str);
	if (env->str - tmp == 0)
		return (false);
	add_token(env, TK_IDENT, tmp, env->str - tmp);
	return (true);
}

static bool	tokenize_number(TokenizeEnv *env)
{
	char	*tmp;

	tmp = env->str;
	env->str = read_number(env->str);
	if (env->str - tmp == 0)
		return (false);
	add_token(env, TK_NUM, tmp, env->str - tmp);
	return (true);
}

static bool	tokenize_char_literal(TokenizeEnv *env)
{
	Token	*tok;

	if (*env->str != '\'')
		return (false);
	tok = add_token(env, TK_CHAR_LIT, ++env->str, 1);
	if (*env->str == '\\')
	{
		env->str += 2;
		tok->len += 1;
	}
	else
	{
		if (*env->str == '\0')
			error_at(env->str - 1, "文字リテラルが終了しませんでした(in)");
		env->str += 1;
	}
	if (*env->str != '\'')
		error_at(tok->str - 1, "文字リテラルが終了しませんでした(end)");
	env->str += 1;
	return (true);
}

static bool	tokenize_str_literal(TokenizeEnv *env, bool is_inc)
{
	bool	is_dq;
	Token	*tok;

	if (*env->str != '\"'
	&& (!is_inc || *env->str != '<'))
		return (false);

	is_dq = *env->str == '\"';
	tok = add_token(env, TK_STR_LIT, env->str + 1, 0);
	tok->is_dq = is_dq;
	env->str++;

	while (*env->str != '\0'
		&& *env->str != '\n'
		&& ((is_dq && *env->str != '\"')
		|| (!is_dq && *env->str != '>')))
	{

		//fprintf(stderr, "%c\n", *env->str);

		if (*env->str == '\\')
		{
			env->str += 1;
			tok->len += 2;
			if (*env->str == '\0')
				error_at(tok->str - 1, "文字列リテラルが終了しませんでした(escape)");
			env->str += 1;
		}
		else
		{
			env->str += 1;
			tok->len += 1;
		}
	}
	if (!(is_dq && *env->str == '\"')
	&& !(!is_dq && *env->str == '>'))
	{
		fprintf(stderr, "err: %c\n", *env->str);
		error_at(tok->str - 1, "文字列リテラルが終了しませんでした(end)");
	}
	env->str += 1;
	return (true);
}

static bool	get_is_include(int wc, Token *last)
{
	if (wc != 2)
		return (false);
	if (last->kind != TK_IDENT
	|| last->len != 7
	|| strncmp(last->str, "include", 7) != 0)
		return (false);
	return (true);
}

static bool	tokenize_directive(TokenizeEnv *env)
{
	int		wc;
	Token	*last;

	if (!env->can_define_dir || *env->str != '#')
		return (false);
	wc = 0;
	last = NULL;
	env->str += 1;
	while (env->str != NULL && *env->str != '\0' && *env->str != '\n')
	{
		wc += 1;
		env->str = skip_space(env->str);
		if (tokenize_ident(env)
			|| tokenize_number(env)
			|| tokenize_str_literal(env, get_is_include(wc, last))
			|| tokenize_char_literal(env)
			|| tokenize_reserved_word(env))
		{
			last = get_last_token(env);
			last->is_directive = true;
			continue ;
		}
		error_at(env->str, "字句解析に失敗しました(directive)");
	}
	if (wc != 0)
	{
		add_token(env, TK_EOD, env->str, 0);
		get_last_token(env)->is_directive = true;
	}
	return (true);
}

Token	*tokenize(char *str)
{
	TokenizeEnv	env;

	env.str = str;
	env.token = NULL;
	env.can_define_dir = true;
	env.str = skip_space(env.str);
	while (env.str != NULL && *env.str != '\0')
	{
		env.str = skip_space(env.str);
		if (tokenize_new_line(&env)
			|| tokenize_ident(&env)
			|| tokenize_str_literal(&env, false)
			|| tokenize_char_literal(&env)
			|| tokenize_reserved_word(&env)
			|| tokenize_number(&env)
			|| tokenize_directive(&env))
			continue ;
		error_at(env.str, "字句解析に失敗しました(tokenize)");
	}
	add_token(&env, TK_EOF, env.str, 0);
	return (env.token);
}
