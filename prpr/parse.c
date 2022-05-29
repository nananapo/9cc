#include "prlib.h"
#include <stdlib.h>
#include <stdbool.h>

Token	*tokens;

static Token	*add_token(ParseEnv *env, TokenKind kind, char *str, int len)
{
	Token	*token;
	Token	*tmp;
	int		i;

	token = malloc(sizeof(Token));
	token->kind = kind;
	token->str = str;
	token->len = len;
	token->next = NULL;
	if (from == NULL)
		env->token = token;
	else
	{
		tmp = env->token;
		while (tmp)
		{
			if (tmp->next == NULL)
				break;
			tmp = tmp->next;
		}
		tmp->next = token;
	}
	return (tmp);
}

static bool	parse_reserved_word(ParseEnv *env)
{
	char	*tmp;
	int		len;
	Token	*tok;

	len = is_reserved_word(env->str);
	if (len == 0)
		return (false);
	if (len == 2 && strncmp(env->str, "//", len) == 0)
	{
		env->str = read_line(env->str);
		parse_new_line(env);
		return (true);
	}
	else if (len == 2 && strncmp(env->str, "/*", len) == 0)
	{
		tmp = env->str;
		env->str = strstr(env->str, "*/");
		if (env->str == NULL)
			error_at(tmp, "/*に対応する*/が見つかりませんでした。");
		env->str += 2;
		return (true);
	}
	else if (len == 2 && strncmp(env->str, "*/", len) == 0)
		error_at(env->str, "*/に対応する/*が見つかりませんでした。");
	add_token(env, TK_RESERVED, env->str, len);
	env->str += len;
	return (true);
}

static void	parse_new_line(ParseEnv *env)
{
	if (*env->str != '\n')
		return ;
	env->can_define_dir = true;
	env->str += 1;
}

static bool	parse_ident(ParseEnv *env)
{
	Token	*tok;

	if (!is_ident_prefix(*env->str))
		return (false);
	tok = add_token(env, TK_IDENT, env->str, 0);
	env->str = read_ident(env->str);
	tok->len = env->str - tok->str;
	return (true);
}

static bool	parse_number(ParseEnv *env)
{
	Token	*tok;

	if (!is_number(*env->str))
		return (false);
	tok = add_token(env, TK_NUM, env->str, 0);
	env->str = read_number(env->str);
	tok->len = env->str - tok->str;
	return (true);
}

Token	*parse(char *str)
{
	ParseEnv	env;
	int			len;

	env.str = skip_space(str);
	env.token = NULL;
	env.can_define_dir = true;
	while (env.str != NULL && *env.str)
	{
		env.str = skip_space(env.str);
		if (*env.str == '\n')
			parse_new_line(&env);
		if (parse_reserved_word(&env);
			continue ;
		if (parse_ident(&env))
			continue ;
		if (parse_number(&env))
			continue ;
		error_at(env->str, "パースに失敗しました");
	}
	add_token(&env, TK_EOF, str, 1);
	return (env->token);
}
