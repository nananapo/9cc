#include "prlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static Node	*create_node(NodeKind kind)
{
	Node	*node;

	node = (Node *)malloc(sizeof(Node));
	node->kind = kind;

	node->codes = NULL;
	node->codes_len = 0;

	node->file_name = NULL;
	node->is_std_include  = false;

	node->macro_name = NULL;
	node->params = NULL;

	node->elif = NULL;
	node->els = NULL;

	node->next = NULL;
	return (node);
}

static Node	*add_node(Node **env, Node *node)
{
	Node	*tmp;

	if (*env == NULL)
		*env = node;
	else
	{
		tmp = *env;
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = node;
	}
	return (node);
}

static bool	consume_eod(ParseEnv *env)
{
	if (env->token == NULL
	|| env->token->kind != TK_EOD)
		return (false);
	env->token = env->token->next;
	return (true);
}

static void	expect_eod(ParseEnv *env)
{
	if (!consume_eod(env))
		error_at(env->token->str, "構文解析に失敗しました(include)");
}

static bool	consume_ident(ParseEnv *env, char *str, bool is_dir)
{
	if (env->token == NULL
	|| env->token->kind != TK_IDENT
	|| env->token->is_directive != is_dir
	|| env->token->len != strlen(str)
	|| strncmp(env->token->str, str, strlen(str)) != 0)
		return (false);
	env->token = env->token->next;
	return (true);
}

static Token	*consume_name(ParseEnv *env, bool is_dir)
{
	Token	*tmp;

	if (env->token == NULL
	|| env->token->kind != TK_IDENT
	|| env->token->is_directive != is_dir)
		return (NULL);
	tmp = env->token;
	env->token = env->token->next;
	return (tmp);
}

static Token	*consume_strlit(ParseEnv *env, bool is_dir)
{
	Token	*tmp;

	if (env->token == NULL
	|| env->token->kind != TK_STR_LIT
	|| env->token->is_directive != is_dir)
		return (NULL);
	tmp = env->token;
	env->token = env->token->next;
	return (tmp);
}

static Token	*consume_keyword(ParseEnv *env, char *str, bool is_dir)
{
	Token	*tmp;

	if (env->token == NULL
	|| env->token->kind != TK_RESERVED
	|| env->token->is_directive != is_dir
	|| env->token->len != strlen(str)
	|| strncmp(env->token->str, str, strlen(str)) != 0)
		return (NULL);
	tmp = env->token;
	env->token = env->token->next;
	return (tmp);
}

static Token	*consume_code(ParseEnv *env, bool is_dir)
{
	Token	*tmp;

	if (env->token == NULL
	|| env->token->is_directive != is_dir)
		return (NULL);
	tmp = env->token;
	env->token = env->token->next;
	return (tmp);
}

static Node	*parse_codes(ParseEnv *env)
{
	Node	*node;
	Token	*tmp;

	if (env->token == NULL || env->token->is_directive)
		return (NULL);
	node = create_node(ND_CODES);
	node->codes = env->token;
	while (true)
	{
		tmp = consume_code(env, false);
		if (tmp == NULL)
			break ;
		node->codes_len += 1;
	}
	return (node);
}

static Node	*parse_include(ParseEnv *env)
{
	Node	*node;
	Token	*lit;

	if (!consume_ident(env, "include", true))
		return (NULL);
	node = create_node(ND_INCLUDE);
	lit = consume_strlit(env, true);
	if (lit == NULL)
		error_at(env->token->str, "ファイル名が必要です(%p %d)", env->token, env->token->kind);
	node->is_std_include = lit->is_dq;
	node->file_name = strndup(lit->str, lit->len);
	expect_eod(env);
	return (node);
}

static Node	*parse_define(ParseEnv *env)
{
	Node	*node;
	Token	*tmp;

	if (!consume_ident(env, "define", true))
		return (NULL);
	node = create_node(ND_DEFINE_MACRO);

	// 名前
	tmp = consume_name(env, true);
	if (tmp == NULL)
		error_at(env->token->str, "名前が見つかりません");
	node->macro_name = strndup(tmp->str, tmp->len);

	// 引数の処理
	tmp = consume_keyword(env, "(", true);
	if (tmp && *(tmp->str - 1) == node->macro_name[strlen(node->macro_name) - 1])
	{
		if (consume_keyword(env, ")", true) == NULL)
		{
			while (true)
			{
				tmp = consume_name(env, true);
				if (tmp == NULL)
					error_at(env->token->str, "引数が見つかりません");
				add_str_elem(&node->params, strndup(tmp->str, tmp->len));
				if (consume_keyword(env, ",", true) == NULL)
				{
					if (consume_keyword(env, ")", true) == NULL)
						error_at(env->token->str, ")が見つかりません");
					break ;
				}
			}
		}
	}

	// 置き換えする単語たちをまとめる
	node->codes = env->token;
	while (consume_eod(env))
	{
		tmp = consume_code(env, true);
		if (tmp == NULL)
			error_at(env->token->str, "不明なトークンです(parse_define)");
		node->codes_len += 1;
	}
	return (node);
}

static Node	*parse_ifdef(ParseEnv *env)
{
	return NULL;
}

static Node	*parse_ifndef(ParseEnv *env)
{
	return NULL;
}

Node	*parse(Token *tok)
{
	ParseEnv	env;
	Node		*node;

	env.token = tok;
	env.node = NULL;
	while (env.token != NULL && env.token->kind != TK_EOF)
	{
		if ((node = parse_codes(&env)) != NULL
		|| (node = parse_include(&env)) != NULL
		|| (node = parse_define(&env)) != NULL
		|| (node = parse_ifdef(&env)) != NULL
		|| (node = parse_ifndef(&env)) != NULL)
		{
			add_node(&env.node, node);
			continue ;
		}
		error_at(env.token->str, "構文解析に失敗しました(parse)");
	}
	return (env.node);
}
