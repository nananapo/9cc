#include "prlib.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

extern GenEnv	gen_env;

static void	codes(Node *node)
{
	int		i;
	Token	*code;

	i = -1;
	code = node->codes;
	while (++i < node->codes_len)
	{
		if (code->kind == TK_STR_LIT)
		{
			printf("\"%s\"", strndup(code->str, code->len));
		}
		else if (code->kind == TK_CHAR_LIT)
		{
			printf("'%s'", strndup(code->str, code->len));
		}
		else
		{
			// TODO define置き換え
			printf("%s ", strndup(code->str, code->len));
		}
		code = code->next;
	}
}

static void	load(char *file_name)
{
	char	*str;
	Token	*tok;
	Node	*node;

	str = read_file(file_name);
	if (str == NULL)
		error("ファイル%sが見つかりせん", file_name);
	tok = tokenize(str);
	node = parse(&tok, 0);
	gen(node);
}

static void	include(Node *node)
{
	char	*file_name;

	file_name = strlit_to_str(node->file_name, strlen(node->file_name));
	if (node->is_std_include)
	{
		// TODO
		return ;
	}
	load(file_name);
}

static void	add_macro(char *name, StrElem *params, Token *codes, int codes_len)
{
	Macro	*tmp;

	tmp = malloc(sizeof(Macro));
	tmp->name = name;
	tmp->params = params;
	tmp->codes = codes;
	tmp->codes_len = codes_len;
	tmp->next = gen_env.macros;
	gen_env.macros = tmp;
}

static Macro	*get_macro(char *name)
{
	Macro	*tmp;

	tmp = gen_env.macros;
	while (tmp != NULL)
	{
		if (strcmp(tmp->name, name) == 0)
			return (tmp);
		tmp = tmp->next;
	}
	return (NULL);
}

static void	define_macro(Node *node)
{
	add_macro(node->macro_name, node->params, node->codes, node->codes_len);
}

static void ifdef(Node *node, bool is_ifdef)
{
	Macro	*macro;

	macro = get_macro(node->macro_name);
	if ((macro != NULL) != is_ifdef)
	{
		if (node->els != NULL)
			gen(node->els);
		// TODO elif
		return ;
	}
	gen(node->stmt);
}	

void	gen(Node *node)
{
	while (node)
	{
		switch (node->kind)
		{
			case ND_INIT:
				break ;
			case ND_CODES:
				codes(node);
				break ;
			case ND_INCLUDE:
				include(node);
				break ;
			case ND_DEFINE_MACRO:
				define_macro(node);
				break ;
			case ND_IFDEF:
			case ND_IFNDEF:
				ifdef(node, node->kind == ND_IFDEF);
				break;
		}
		node = node->next;
	}
}
