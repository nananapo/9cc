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
	node->filename = NULL;
	node->name = NULL;
	node->elif = NULL;
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

static Node	*parse_codes(ParseEnv *env)
{
	Node	*node;

	if (env->token == NULL || env->token->is_directive)
		return (NULL);
	node = create_node(ND_CODES);
	while (env->token != NULL && !env->token->is_directive)
	{
		node->codes_len += 1;
		env->token = env->token->next;
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

static bool	consume_ident(ParseEnv *env, char *str, bool is_dir)
{
	if (env->token == NULL
	|| env->token->kind != TK_IDENT
	|| env->token->is_directive != is_dir
	|| strncmp(env->token->str, str, strlen(str)) != 0)
		return (false);
	env->token = env->token->next;
	return (true);
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

static Node	*parse_include(ParseEnv *env)
{
	Node	*node;
	Token	*lit;

	if (!consume_ident(env, "include", true))
		return (NULL);
	node = create_node(ND_INCLUDE);
	lit = consume_strlit(env, true);
	if (lit == NULL)
		error_at(env->token->str, "ファイル名が必要です");
	node->is_std_include = lit->is_dq;
	node->filename = strndup(lit->str, lit->len);
	if (!consume_eod(env))
		error_at(env->token->str, "構文解析に失敗しました(include)");
	return (node);
}

/*
static bool	parse_define(ParseEnv *env)
{
	
}

static bool	parse_ifdef(ParseEnv *env)
{
	
}

static bool	parse_ifndef(ParseEnv *env)
{
	
}
*/

Node	*parse(Token *tok)
{
	ParseEnv	env;
	Node		*node;

	env.token = tok;
	env.node = NULL;
	while (env.token != NULL && env.token->kind != TK_EOF)
	{
		printf("%d\n", env.token->kind);

		if ((node = parse_codes(&env)) != NULL
		|| (node = parse_include(&env)) != NULL)
		{
			add_node(&env.node, node);
			printf("%p\n", env.token);
			continue ;
		}
		/*
		|| parse_define(env)
		|| parse_ifdef(env)
		|| parse_ifndef(env))
			continue ;*/
		error_at(env.token->str, "構文解析に失敗しました(parse)");
	}
	return (env.node);
}
