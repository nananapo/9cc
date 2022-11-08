#include "9cc.h"
#include "parse.h"
#include "charutil.h"
#include "stack.h"
#include "mymath.h"

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

t_deffunc	*get_function_by_name(char *name, int len);
bool		consume_enum_key(t_type **type, int *value);
t_str_elem	*get_str_literal(char *str, int len);
bool consume_float(float *result);

static t_node	*call(t_node *funcnode, t_token *ident);
static t_node	*read_postfix_expression(t_node *node);
static t_node	*primary(void);
static t_node	*unary(void);
static t_node	*mul(void);
static t_node	*add(void);
static t_node	*shift(void);
static t_node	*relational(void);
static t_node	*equality(void);
static t_node	*bitwise_and(void);
static t_node	*bitwise_or(void);
static t_node	*bitwise_xor(void);
static t_node	*conditional_and(void);
static t_node	*conditional_or(void);
static t_node	*conditional_op(void);
static t_node	*assign(void);
static t_node	*expr(bool comma);
static t_node	*read_ifblock(void);
static t_node	*stmt(void);
static t_node	*expect_constant(t_type *type);
static void	global_var(t_type *type, t_token *ident, bool is_extern, bool is_static);
t_type		*read_struct_block(t_token *ident);
t_type		*read_enum_block(t_token *ident);
t_type		*read_union_block(t_token *ident);
static void	read_typedef(void);
static void	filescope(void);
void		parse(void);

// main
extern t_token			*g_token;
extern t_deffunc		*g_func_defs[1000];
extern t_deffunc		*g_func_protos[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals[1000];
extern t_defstruct		*g_struct_defs[1000];
extern t_defenum		*g_enum_defs[1000];
extern t_defunion		*g_union_defs[1000];
extern t_linked_list	*g_type_alias;

t_deffunc	*get_function_by_name(char *name, int len)
{
	int			i;
	t_deffunc	*tmp;

	i = 0;
	while (g_func_defs[i])
	{
		tmp = g_func_defs[i];
		if (tmp->name_len == len && strncmp(tmp->name, name, len) == 0)
			return (tmp);
		i++;
	}
	i = 0;
	while (g_func_protos[i])
	{
		tmp = g_func_protos[i];
		if (tmp->name_len == len && strncmp(tmp->name, name, len) == 0)
			return (tmp);
		i++;
	}
	return NULL;
}

t_node	*new_node(t_nodekind kind, t_node *lhs, t_node *rhs, char *source)
{
	t_node	*node;

	node					= calloc(1, sizeof(t_node));
	node->kind				= kind;
	node->lhs				= lhs;
	node->rhs				= rhs;
	node->analyze_source	= source;
	return (node);
}

t_node	*new_node_num(int val, char *source)
{
	t_node	*node;

	node		= new_node(ND_NUM, NULL, NULL, source);
	node->val	= val;
	node->type	= new_primitive_type(TY_INT);
	return (node);
}

t_node	*new_node_float(float val, char *source)
{
	t_node	*node;

	node		= new_node(ND_FLOAT, NULL, NULL, source);
	node->val_float	= val;
	node->type	= new_primitive_type(TY_FLOAT);
	return (node);
}

// 文字列リテラルを保存してオブジェクトを取得する
// 既に同じ文字列があるならそのオブジェクトを取得する
t_str_elem	*get_str_literal(char *str, int len)
{
	t_str_elem	*tmp;
	t_str_elem	*elem;
	int			i;

	// find same literal
	for (i = 0; g_str_literals[i] != NULL; i++)
	{
		tmp = g_str_literals[i];
		if (len == tmp->len && strncmp(tmp->str, str, len) == 0)
			return (tmp);
	}

	// save 
	elem = calloc(1, sizeof(t_str_elem));
	elem->str = str;
	elem->len = len;
	elem->index = i;
	g_str_literals[i] = elem;
	return (elem);
}

static t_node	*call(t_node *funcnode, t_token *ident)
{
	t_node		*node;

	node						= new_node(ND_CALL, NULL, NULL, g_token->str);
	if (ident != NULL)
	{
		node->analyze_funccall_name		= ident->str;
		node->analyze_funccall_name_len	= ident->len;
		node->funcdef					= get_function_by_name(ident->str, ident->len);
	}
	node->lhs					= funcnode;
	node->funccall_argcount		= 0;

	// read arguments
	for (;;)
	{	
		if (consume(")"))
			break;

		if (node->funccall_argcount != 0 && !consume(","))
			error_at(g_token->str, "トークンが,ではありません");

		node->funccall_args[node->funccall_argcount++] = expr(false);
	}
	return (node);
}

// 添字, 関数呼び出し
static t_node	*read_postfix_expression(t_node *node)
{
	char		*source;
	t_token		*ident;

	source = g_token->str;
	// call func
	if (consume("["))
	{
		node = new_node(ND_ADD, node, expr(true), source);
		node = new_node(ND_DEREF, node, NULL, source);
		if (!consume("]"))
			error_at(g_token->str, "%s");
	}
	else if (consume("("))
	{
		node = call(node, NULL);
	}
	else if (consume("."))
	{
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です\n (read_postfix_expr)");
		node							= new_node(ND_MEMBER_VALUE, node, NULL, ident->str);
		node->analyze_member_name		= ident->str;
		node->analyze_member_name_len	= ident->len;
		node							= read_postfix_expression(node);
	}
	else if (consume("->"))
	{
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です\n (read_postfix_expr)");
		node							= new_node(ND_MEMBER_PTR_VALUE, node, NULL, ident->str);
		node->analyze_member_name		= ident->str;
		node->analyze_member_name_len	= ident->len;
		node							= read_postfix_expression(node);
	}
	else if (consume("++"))
	{
		node = new_node(ND_COMP_ADD, node, new_node_num(1, source), source);
		node = new_node(ND_SUB, node, new_node_num(1, source), source);
	}
	else if (consume("--"))
	{
		node = new_node(ND_COMP_SUB, node, new_node_num(1, source), source);
		node = new_node(ND_ADD, node, new_node_num(1, source), source);
	}
	else
		return (node);

	return (read_postfix_expression(node));
}

static t_node *primary(void)
{
	t_token	*tok;
	t_node	*node;
	int 	number;
	float 	number_float;
	t_type	*enum_type;
	char	*source;

	source = g_token->str;

	// 括弧
	if (consume("("))
	{
		node = expr(true);
		node = new_node(ND_PARENTHESES, node, NULL, node->analyze_source);
		expect(")");
		return (read_postfix_expression(node));
	}

	// enumの値
	if (consume_enum_key(&enum_type, &number))
	{
		node = new_node_num(number, source);
		node->type = enum_type;
		return (read_postfix_expression(node));
	}

	// 変数か関数呼び出し
	tok = consume_ident();
	if (tok)
	{
		if (consume("("))
			return (read_postfix_expression(call(NULL, tok)));
		node			= new_node(ND_ANALYZE_VAR, NULL, NULL, tok->str);
		node->analyze_var_name		= tok->str;
		node->analyze_var_name_len	= tok->len;
		return (read_postfix_expression(node));
	}

	// string
	tok = consume_str_literal();
	if (tok)
	{
		node = new_node(ND_STR_LITERAL, NULL, NULL, tok->str);
		node->def_str = get_str_literal(tok->str, tok->len);
		return (read_postfix_expression(node));
	}

	// char
	tok = consume_char_literal();
	if (tok)
	{
		number = char_to_int(tok->str, tok->strlen_actual);
		if (number == -1)
			error_at(tok->str, "不明なエスケープシーケンスです (primary)");
		node = new_node_num(number, tok->str);
		node->type = new_primitive_type(TY_CHAR);
		return (read_postfix_expression(node));
	}

	if (consume_float(&number_float))
	{
		node = new_node_float(number_float, source);
		return (node);
	}

	// 数
	if (!consume_number(&number))
		error_at(g_token->str, "数字が必要です");
	node = new_node_num(number, source);

	return (read_postfix_expression(node));
}

static t_node	*parse_sizeof(void)
{
	t_node	*node;
	t_type	*type;
	t_token	*token;
	char	*source;

	if (consume("("))
	{
		source	= g_token->str;
		if (!consume_type(&type, &token, NULL, NULL))
			node = new_node(ND_SIZEOF, unary(), NULL, source);
		else
			node = new_node_num(get_type_size(type), source);

		if (!consume(")"))
			error_at(g_token->str, ")が必要です");
	}
	else
	{
		node = unary();
		node = new_node(ND_SIZEOF, node, NULL, node->analyze_source);
	}
	return (node);
}

static t_node	*unary(void)
{
	char	*source;

	source = g_token->str;
	if (consume("+"))
		return (new_node(ND_ADD_UNARY, unary(), NULL, source));
	else if (consume("-"))
		return (new_node(ND_SUB_UNARY, unary(), NULL, source));
	else if (consume("*"))
		return (new_node(ND_DEREF, unary(), NULL, source));
	else if (consume("&"))
		return (new_node(ND_ADDR, unary(), NULL, source));
	else if (consume("++"))
		return (new_node(ND_COMP_ADD, unary(), new_node_num(1, source), source));
	else if (consume("--"))
		return (new_node(ND_COMP_SUB, unary(), new_node_num(1, source), source));
	else if (consume("!"))
		return (new_node(ND_EQUAL, unary(), new_node_num(0, source), source));
	else if (consume("~"))
		return (new_node(ND_BITWISE_NOT, unary(), NULL, source));
	else if (consume_kind(TK_SIZEOF))
		return (parse_sizeof());
	return (primary());
}

static t_node	*cast_expression(void)
{
	t_token	*tmp;
	t_token	*ident;
	t_type	*type;
	t_node	*node;

	tmp = g_token;
	if (consume("("))
	{
		if (!consume_type(&type, &ident, NULL, NULL))
		{
			g_token = tmp;
			return (unary());
		}
		expect(")");
		node = cast_expression();
		node = new_node(ND_CAST, node, NULL, node->analyze_source);
		node->type = type;
		return (node);
	}
	return (unary());
}

static t_node	*mul(void)
{
	t_node	*node;
	char	*source;

	node = cast_expression();
	for (;;)
	{
		source = g_token->str;
		if (consume("*"))
			node = new_node(ND_MUL, node, cast_expression(), source);
		else if (consume("/"))
			node = new_node(ND_DIV, node, cast_expression(), source);
		else if (consume("%"))
			node = new_node(ND_MOD, node, cast_expression(), source);
		else
			break ;
	}
	return (node);
}

static t_node	*add(void)
{
	t_node	*node;
	char	*source;

	node = mul();
	for (;;)
	{
		source = g_token->str;
		if (consume("+"))
			node = new_node(ND_ADD, node, mul(), source);
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul(), source);
		else
			break ;
	}
	return (node);
}

static t_node	*shift(void)
{
	t_node	*node;
	char	*source;

	node	= add();
	source	= g_token->str;
	if (consume("<<"))
		node = new_node(ND_SHIFT_LEFT, node, shift(), source);
	else if (consume(">>"))
		node = new_node(ND_SHIFT_RIGHT, node, shift(), source);
	return (node);
}

static t_node	*relational(void)
{
	t_node	*node;
	char	*source;

	node = shift();
	for (;;)
	{
		source = g_token->str;
		if (consume("<"))
			node = new_node(ND_LESS, node, shift(), source);
		else if (consume("<="))
			node = new_node(ND_LESSEQ, node, shift(), source);
		else if (consume(">"))
			node = new_node(ND_LESS, shift(), node, source);
		else if (consume(">="))
			node = new_node(ND_LESSEQ, shift(), node, source);
		else
			break ;
	}
	return (node);
}

static t_node *equality(void)
{
	t_node	*node;
	char	*source;

	node = relational();
	for (;;)
	{
		source = g_token->str;
		if (consume("=="))
			node = new_node(ND_EQUAL, node, relational(), source);
		else if (consume("!="))
			node = new_node(ND_NEQUAL, node, relational(), source);
		else
			break ;
	}
	return (node);
}

static t_node	*bitwise_and(void)
{
	t_node	*node;
	char	*source;

	node	= equality();
	source	= g_token->str;
	if (consume("&"))
		node = new_node(ND_BITWISE_AND, node, bitwise_and(), source);
	return (node);
}


static t_node	*bitwise_xor(void)
{
	t_node	*node;
	char	*source;

	node	= bitwise_and();
	source	= g_token->str;
	if (consume("^"))
		node = new_node(ND_BITWISE_XOR, node, bitwise_xor(), source);
	return (node);
}

static t_node	*bitwise_or(void)
{
	t_node	*node;
	char	*source;

	node	= bitwise_xor();
	source	= g_token->str;
	if (consume("|"))
		node = new_node(ND_BITWISE_OR, node, bitwise_or(), source);
	return (node);
}

static t_node	*conditional_and(void)
{
	t_node	*node;
	char	*source;

	node	= bitwise_or();
	source	= g_token->str;
	if (consume("&&"))
		node = new_node(ND_COND_AND, node, conditional_and(), source);
	return (node);
}

static t_node	*conditional_or(void)
{
	t_node	*node;
	char	*source;

	node	= conditional_and();
	source	= g_token->str;
	if (consume("||"))
		node = new_node(ND_COND_OR, node, conditional_or(), source);
	return (node);
}

static t_node	*conditional_op(void)
{
	t_node	*node;
	char	*source;

	node	= conditional_or();
	source	= g_token->str;
	if (consume("?"))
	{
		node = new_node(ND_COND_OP, node, expr(true), source);
		if (!consume(":"))
			error_at(g_token->str, ":が必要です");
		node->els = expr(true);
	}
	return (node);
}

static t_node	*assign(void)
{
	t_node	*node;
	char	*source;

	node = conditional_op();
	source = g_token->str;
	if (consume("="))
		node = new_node(ND_ASSIGN, node, assign(), source);
	else if (consume("+="))
		node = new_node(ND_COMP_ADD, node, assign(), source);
	else if (consume("-="))
		node = new_node(ND_COMP_SUB, node, assign(), source);
	else if (consume("*="))
		node = new_node(ND_COMP_MUL, node, assign(), source);
	else if (consume("/="))
		node = new_node(ND_COMP_DIV, node, assign(), source);
	else if (consume("%="))
		node = new_node(ND_COMP_MOD, node, assign(), source);
	return (node);
}

static t_node	*expr(bool comma)
{
	t_node	*node;

	node = assign();
	if (comma && consume(","))
	{	
		node		= new_node(ND_BLOCK, node, NULL, node->analyze_source);
		node->rhs	= new_node(ND_BLOCK, expr(true), NULL, NULL);
		node->rhs->analyze_source = node->rhs->lhs->analyze_source;
		return (node);
	}
	return (node);
}

// ifの後ろの括弧から読む
static t_node	*read_ifblock(void)
{
	t_node	*node;
	char	*source;

	source = g_token->str;
	if (!consume("("))
		error_at(g_token->str, "(ではないトークンです");
	node = new_node(ND_IF, expr(true), NULL, source);

	if (!consume(")"))
		error_at(g_token->str, ")ではないトークンです");
	node->rhs = stmt();

	if (consume_kind(TK_ELSE))
	{
		if (consume_kind(TK_IF))
			node->elsif = read_ifblock();
		else
			node->els = stmt();
	}
	return (node);
}

static t_node	*consume_const_local(t_type *type)
{
	t_node	*first;
	t_node	*tmp;
	t_node	*last;

	first = NULL;
	last = NULL;
	if (is_pointer_type(type) && consume("{"))
	{
		for (;;)
		{
			tmp = new_node(ND_NONE, expect_constant(type->ptr_to), NULL, NULL);
			if (last == NULL)
				first = tmp;
			else
				last->global_array_next = tmp;
			last = tmp;
			if (consume("}"))
				break ;
			if (!consume(","))
				error_at(g_token->str, ",が必要です");
		}
	}
	return (first);
}

// TODO 条件の中身がintegerか確認する
static t_node	*stmt(void)
{
	t_node	*node;
	t_type	*type;
	t_token *ident;
	int		number;
	t_node	*start;
	char	*source;

	source = g_token->str;
	// return ;
	// return lhs;
	if (consume_kind(TK_RETURN))
	{
		if (consume(";"))
			node = new_node(ND_RETURN, NULL, NULL, source);
		else
		{
			node = new_node(ND_RETURN, expr(true), NULL, source);
			expect_semicolon();
		}
		return (node);
	}
	else if (consume_kind(TK_IF))
		return (read_ifblock());
	else if (consume_kind(TK_WHILE))
	{
		if (!consume("("))
			error_at(g_token->str, "(ではないトークンです");
		node = new_node(ND_WHILE, expr(true), NULL, source);
		if (!consume(")"))
			error_at(g_token->str, ")ではないトークンです");
		if (!consume(";"))
			node->rhs = stmt();
		return node;
	}
	// do lhs while (rhs)
	else if (consume_kind(TK_DO))
	{
		node = new_node(ND_DOWHILE, stmt(), NULL, source);
		if (!consume_kind(TK_WHILE))
			error_at(g_token->str, "whileが必要です");
		if (!consume("("))
			error_at(g_token->str, "(ではないトークンです");
		if (!consume(";"))
			node->rhs = expr(true);
		if (!consume(")"))
			error_at(g_token->str, ")ではないトークンです");
	}
	// for (0; 1; 2) lhs
	else if (consume_kind(TK_FOR))
	{
		if (!consume("("))
			error_at(g_token->str, "(ではないトークンです");
		node = new_node(ND_FOR, NULL, NULL, source);
		if (!consume(";")) 
		{
			node->for_expr[0] = expr(true);
			expect_semicolon();
		}
		if (!consume(";"))
		{
			node->for_expr[1] = expr(true);
			expect_semicolon();
		}
		if (!consume(")"))
		{
			node->for_expr[2] = expr(true);
			if(!consume(")"))
				error_at(g_token->str, ")ではないトークンです");
		}
		if (!consume(";"))
			node->lhs = stmt();
		return (node);
	}
	else if (consume_kind(TK_SWITCH))
	{
		if (!consume("("))
			error_at(g_token->str, "(ではないトークンです");
		node = new_node(ND_SWITCH, expr(true), NULL, source);
		if (!consume(")"))
			error_at(g_token->str, ")ではないトークンです");
		node->rhs = stmt();
		return (node);
	}
	else if (consume_kind(TK_CASE))
	{
		if (!consume_number(&number))
		{
			if (!consume_enum_key(NULL, &number))
			{
				ident = consume_char_literal();
				if (ident == NULL)
					error_at(g_token->str, "定数が必要です");
				number = char_to_int(ident->str, ident->strlen_actual);
			}
		}
		if (!consume(":"))
			error_at(g_token->str, ":が必要です");
		node		= new_node_num(number, source);
		node->kind	= ND_CASE;
		return (node);
	}
	else if (consume_kind(TK_BREAK))
	{
		expect_semicolon();
		return (new_node(ND_BREAK, NULL, NULL, source));
	}
	else if (consume_kind(TK_CONTINUE))
	{
		expect_semicolon();
		return (new_node(ND_CONTINUE, NULL, NULL, source));
	}
	else if (consume_kind(TK_DEFAULT))
	{
		if (!consume(":"))
			error_at(g_token->str, ":が必要です");
		return (new_node(ND_DEFAULT, NULL, NULL, source));
	}
	else if (consume("{"))
	{
		node	= new_node(ND_BLOCK, NULL, NULL, source);
		start	= node;
		while (!consume("}"))
		{
			node->lhs	= stmt();
			node->rhs	= new_node(ND_BLOCK, NULL, NULL, node->lhs->analyze_source);
			node		= node->rhs;
		}
		return (start);
	}
	else if (consume_type(&type, &ident, NULL, NULL))
	{
		if (ident == NULL)
		{
			node = new_node(ND_NONE, NULL, NULL, source);
		}
		else
		{
			node = new_node(ND_VAR_DEF, NULL, NULL, ident->str);
			node->type = type;
			node->analyze_var_name = ident->str;
			node->analyze_var_name_len = ident->len;

			// 宣言と同時に代入
			if (consume("="))
			{
				node->lvar_const = consume_const_local(node->type);
				if (node->lvar_const == NULL)
					node->lvar_assign = expr(true);
			}
		}
	}
	else
	{
		node = expr(true);
	}
	
	if(!consume(";"))
		error_at(g_token->str, ";ではないトークン(Kind : %d , %s)です", g_token->kind, my_strndup(g_token->str, g_token->len));

	return (node);
}

static t_node	*expect_constant(t_type *type)
{
	t_node	*node;
	int		number;
	t_token	*tok;
	char	*source;

	t_node	*first;
	t_node	*last;
	t_node	*tmp;

	source = g_token->str;
	if (is_integer_type(type) && consume_number(&number))
	{
		node = new_node_num(number, source);
	}
	// とりあえずマイナスも読めるように....
	else if (is_integer_type(type) && consume("-"))
	{
		if (is_integer_type(type) && consume_number(&number))
		{
			node = new_node_num(number, source);
		}
		else
		{
			error("Error");
			node = NULL;
		}
	}
	// char *でstrlit
	else if (type_equal(type, new_type_ptr_to(new_primitive_type(TY_CHAR)))
			&& (tok = consume_str_literal()) != NULL)
	{
		node = new_node(ND_STR_LITERAL, NULL, NULL, source);
		node->def_str = get_str_literal(tok->str, tok->len);
	}
	// pointerで数字
	else if (is_pointer_type(type) && consume_number(&number))
	{
		node = new_node_num(number, source);
	}
	// char でcharlit
	else if (type_equal(type, new_primitive_type(TY_CHAR)) 
			&& (tok = consume_char_literal()) != NULL)
	{
		number = char_to_int(tok->str, tok->strlen_actual);
		if (number == -1)
			error_at(tok->str, "不明なエスケープシーケンスです (expect_constant)");
		node = new_node_num(number, source);
	}
	else if (is_pointer_type(type) && consume("{"))
	{
		first = NULL;
		last = NULL;
		for (;;)
		{
			tmp = new_node(ND_NONE, expect_constant(type->ptr_to), NULL, NULL);
			if (last == NULL)
				first = tmp;
			else
				last->global_array_next = tmp;
			last = tmp;
			if (consume("}"))
				break ;
			if (!consume(","))
				error_at(g_token->str, ",が必要です");
		}
		node = first;
	}
	else
	{
		node = NULL;
		error_at(g_token->str, "定数が必要です(c)");
	}
	return (node);
}

static void	global_var(t_type *type, t_token *ident, bool is_extern, bool is_static)
{
	int			i;
	t_defvar	*defvar;

	// 型だけ
	if (ident == NULL)
	{
		expect_semicolon();
		return ;
	}

	defvar				= calloc(1, sizeof(t_defvar));
	defvar->name		= ident->str;
	defvar->name_len	= ident->len;
	defvar->type		= type;
	defvar->is_extern	= is_extern;
	defvar->is_static	= is_static;

	// 保存
	for (i = 0; g_global_vars[i] != NULL; i++);
	g_global_vars[i] = defvar;

	// 代入
	if (consume("="))
	{
		if (is_extern)
			error_at(ident->str, "externしている変数に代入はできません");
		defvar->assign = expect_constant(defvar->type);
	}

	expect_semicolon();
}

// {以降を読む
t_type	*read_struct_block(t_token *ident)
{
	t_type		*type;
	t_defstruct	*def;
	t_member	*elem;
	t_member	**tmp_elem;
	int			i;

	def				= calloc(1, sizeof(t_defstruct));
	def->name		= ident->str;
	def->name_len	= ident->len;
	def->is_declared= false;
	tmp_elem		= &def->members;

	// 保存
	for (i = 0; g_struct_defs[i]; i++) ;
	g_struct_defs[i] = def;

	while (1)
	{
		if (consume("}"))
			break ;

		if (!consume_type(&type, &ident, NULL, NULL))
			error_at(g_token->str, "型宣言が必要です\n (read_union_block)");
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です\n (read_union_block)");
		expect_semicolon();

		elem			= calloc(1, sizeof(t_member));
		elem->name		= ident->str;
		elem->name_len	= ident->len;
		elem->type		= type;
		elem->is_struct	= true;
		elem->strct		= def;
		*tmp_elem	= elem;
		tmp_elem	= &elem->next;

		if ((type->ty == TY_STRUCT && !type->strct->is_declared)
		|| (type->ty == TY_UNION && !type->unon->is_declared))
			error_at(ident->str, "型のサイズが確定していません");
	}

	def->is_declared = true;
	type = new_struct_type(def->name, def->name_len);
	return (type);
}

// {以降を読む
t_type	*read_enum_block(t_token *ident)
{
	int			i;
	t_defenum	*def;
	t_type		*type;

	def				= calloc(1, sizeof(t_defenum));
	def->name		= ident->str;
	def->name_len	= ident->len;
	def->kind_len	= 0;

	// 保存
	for (i = 0; g_enum_defs[i]; i++) ;
	g_enum_defs[i] = def;

	while (1)
	{
		if (consume("}"))
			break ;

		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が見つかりません");

		def->kinds[def->kind_len++] = my_strndup(ident->str, ident->len);

		if (!consume(","))
		{
			if (!consume("}"))
				error_at(g_token->str, "}が見つかりません");
			break ;
		}
	}

	type = new_enum_type(def->name, def->name_len);
	return (type);
}

// {以降を読む
t_type	*read_union_block(t_token *ident)
{
	t_defunion	*def;
	t_member	*elem;
	t_type		*type;
	int			i;

	def				= calloc(1, sizeof(t_defunion));
	def->name		= ident->str;
	def->name_len	= ident->len;
	def->is_declared= false;

	// 保存
	for (i = 0; g_union_defs[i]; i++) ;
	g_union_defs[i] = def;

	// 要素を追加 & 最大のサイズを取得
	while (1)
	{
		if (consume("}"))
			break;

		if (!consume_type(&type, &ident, NULL, NULL))
			error_at(g_token->str, "型宣言が必要です\n (read_union_block)");
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です\n (read_union_block)");
		expect_semicolon();

		elem			= calloc(1, sizeof(t_member));
		elem->name		= ident->str;
		elem->name_len	= ident->len;
		elem->type		= type;
		elem->next		= def->members;
		elem->is_struct	= false;
		elem->unon		= def;
		def->members = elem;

		if ((type->ty == TY_STRUCT && !type->strct->is_declared)
		|| (type->ty == TY_UNION && !type->unon->is_declared))
			error_at(ident->str, "型のサイズが確定していません");
	}

	def->is_declared = true;
	type = new_union_type(def->name, def->name_len);
	return (type);
}

static void	funcdef(t_deffunc *def)
{
	int			i;

	// func_defsに代入
	if (consume(";"))
	{
		def->is_prototype = true;
		for (i = 0; g_func_protos[i] != NULL; i++);
		g_func_protos[i] = def;
	}
	else
	{
		def->is_prototype = false;
		for (i = 0; g_func_defs[i] != NULL; i++);
		g_func_defs[i] = def;
		def->stmt = stmt();
	}
	
	debug(" func %s created", my_strndup(def->name, def->name_len));
}

static void	read_typedef(void)
{
	t_type			*type;
	t_token			*token;
	t_typedefpair	*pair;

	if (!consume_type(&type, &token, NULL, NULL))
		error_at(g_token->str, "型宣言が必要です\n (read_typedef)");

	if (token == NULL)
		error_at(g_token->str, "識別子が必要です");

	pair			= malloc(sizeof(t_typedefpair));
	pair->name		= token->str;
	pair->name_len	= token->len;
	pair->type		= type;

	linked_list_insert(g_type_alias, pair);
	expect_semicolon();
}

static void	filescope(void)
{
	t_token	*ident;
	t_type	*type;
	bool	is_static;
	bool	is_extern;

	// typedef
	if (consume_kind(TK_TYPEDEF))
	{
		read_typedef();
		return ;
	}

	if (consume_kind(TK_INLINE))
	{
		// 無視
	}

	if (!consume_type(&type, &ident, &is_static, &is_extern))
		error_at(g_token->str, "error : filescope");

	if (type->ty == TY_FUNCTION)
		funcdef(type->funcdef);
	else
		global_var(type, ident, is_extern, is_static);
}

/*
static t_storage_class	consume_storage_class_specifier(void)
{
	t_storage_class	r;

	r = SC_NONE;
	switch (g_token->kind)
	{
		case TK_AUTO:
			r = SC_AUTO;
			break ;
		case TK_REGISTER:
			r = SC_REGISTER;
			break ;
		case TK_STATIC:
			r = SC_STATIC;
			break ;
		case TK_EXTERN:
			r = SC_EXTERN;
			break ;
		case TK_TYPEDEF:
			r = SC_TYPEDEF;
			break ;
		default:
	}
	if (r != SC_NONE)
		consume_any();
	return (r);
}

static t_declaration_specifiers	*consume_declaration_specifiers(void)
{
	t_token						*tmp;
	t_declaration_specifiers	*dec;
	t_storage_class				storage_class;
	t_declaration_specifiers	*spec1;
	t_type_specifier			*type_specifier;
	t_declaration_specifiers	*spec2;
	ttype_qualifier				*type_qualifier;
	t_declaration_specifiers	*spec3;

	tmp = g_token;
	storage_class = consume_storage_class_specifier();
	if (storage_class = SC_NONE)
		return (NULL);
	spec1			= consume_declatation_specifiers();
	type_specifier	= expect_type_specifier();
	if (type_specifier == NULL)
	{
		g_token = tmp;
		return (NULL);
	}
	spec2			= consume_declatation_specifiers();
	type_qualifier	= consume_type_qualifier();
	if (type_qualifier == NULL)
	{
		g_token = tmp;
		return (NULL);
	}
	spec3			= consume_declatation_specifiers();

	dec = calloc(1, sizeof(t_declaration_specifiers));
	dec->storage = storage_class;
	dec->spec1 = spec1;
	dec->spec2 = spec2;
	dec->spec3 = spec3;
}

static bool consume_function_definition(void)
{
	t_token						*tmp;
	t_function_definition		*function_definition;
	t_declaration_specifiers	*declaration_specifiers;
	t_declarator				*declarator;
	t_declaration_list			*declaration_list;
	t_compound_statement		*compound_statement;

	tmp	= g_token;
	declaration_specifiers	= consume_declaration_specifiers();
	declarator				= consume_declarator();
	if (declarator == NULL)
	{
		g_token = tmp;
		return (false);
	}
	declaration_list		= consume_declaration_list();

	function_definition	 = calloc(1, sizeof(t_function_definition));
	function_definition->declaration_specifiers	= declaration_specifiers;
	function_definition->declarator				= declarator;
	function_definition->declaration_lists		= declaration_list;
	function_definition->compound_statement 	= compound_statement;

	// TODO 保存する
}

static void	external_declaration(void)
{
	if (consume_function_definition())
		return ;
	else if (consume_declaration())
		return ;
}
*/

void	parse(void)
{
	while (!at_eof())
		filescope();
}
