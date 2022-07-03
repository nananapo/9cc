#include "prlib.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

static GenEnv	gen_env;

static char	*currentdir;

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

static void	apply_macro(Macro *macro, Token **tok)
{
	Node	*node;

	// TODO 関数の置き換え
	// TODO さらに置き換え
	//名前をスキップ
	*tok = (*tok)->next;
	// TODO  置き換える
	node = create_node(ND_CODES);
	node->codes = macro->codes;
	node->codes_len = macro->codes_len;
	gen(node);
}

static bool	consume_reserved(Token *tok, char *str)
{
	int	len;

	len = strlen(str);
	if (tok->kind != TK_RESERVED
	|| tok->len != len
	|| strncmp(tok->str, str, len) != 0)
		return (false);
	return (true);
}

static void	print_line(void)
{
	int	i;

	printf("\n");
	i = -1;
	while (++i < gen_env.nest_count)
		printf("  ");
	gen_env.print_count = 0;
}

static void	codes(Node *node)
{
	int		i;
	Token	*code;
	Macro	*mactmp;	

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
		else if (code->kind == TK_IDENT)
		{
			mactmp = get_macro(strndup(code->str, code->len));
			if (mactmp == NULL)
			{
				printf("%s ", strndup(code->str, code->len));
			}
			else
			{
				apply_macro(mactmp, &code);
				continue ;
			}
		}
		else if (code->kind == TK_RESERVED)
		{
			if (consume_reserved(code, "{"))
			{
				print_line();
				printf("{");
				gen_env.nest_count += 1;
				print_line();
			}
			else if (consume_reserved(code, "}"))
			{
				gen_env.nest_count -= 1;
				print_line();
				printf("}");
				print_line();
			}
			else if (consume_reserved(code, ";"))
			{
				printf(";");
				print_line();
			}
			else
			{
				printf("%s ", strndup(code->str, code->len));
			}
		}
		else
		{
			printf("%s", strndup(code->str, code->len));
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
	{
		printf("%s\n", file_name);
		error("^ ファイルが見つかりませんでした");
	}
	tok = tokenize(str);
	node = parse(&tok, 0);
	gen(node);
}

static void	include(Node *node)
{
	char	*file_name;
	char	str[10000];

	file_name = strlit_to_str(node->file_name, strlen(node->file_name));
	if (node->is_std_include)
	{
		str[0] = '\0';
		strcat(str, gen_env.stddir);
		strcat(str, file_name);
		load(str);
	}
	else
	{
		load(file_name);
	}
}

static void	define_macro(Node *node)
{
	// TODO 被りチェック
	add_macro(node->macro_name, node->params, node->codes, node->codes_len);
}

static void	undef_macro(Node *node)
{
	Macro	*tmp;
	Macro	*last;

	last = NULL;
	tmp = gen_env.macros;
	while (tmp != NULL)
	{
		if (strlen(node->macro_name) != strlen(tmp->name)
		|| strncmp(node->macro_name, tmp->name, strlen(node->macro_name)) != 0)
		{
			last = tmp;
			tmp = tmp->next;
			continue ;
		}
		if (last == NULL)
			gen_env.macros = tmp->next;
		else
			last->next = tmp->next;
		break ;
	}
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
			case ND_UNDEF:
				undef_macro(node);
				break ;
			case ND_IFDEF:
			case ND_IFNDEF:
				ifdef(node, node->kind == ND_IFDEF);
				break ;
		}
		node = node->next;
	}
}

void	set_currentdir(char *stddir, char *filename)
{
	gen_env.stddir = stddir;
	currentdir = getdir(filename);
}
