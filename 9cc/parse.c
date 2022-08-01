#include "9cc.h"
#include "parse.h"
#include "charutil.h"
#include "stack.h"

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

t_deffunc	*get_function_by_name(char *name, int len);
Node	*new_node(NodeKind kind, Node *lhs, Node *rhs);
Node	*new_node_num(int val);
bool	consume_enum_key(Type **type, int *value);
t_str_elem	*get_str_literal(char *str, int len);


SBData	*sbdata_new(bool isswitch, int start, int end);
void	sb_forwhile_start(int startlabel, int endlabel);
void	sb_switch_start(Type *type, int endlabel, int defaultLabel);
SBData	*sb_end(void);
SBData	*sb_peek(void);
SBData	*sb_search(bool	isswitch);

Node	 *cast(Node *node, Type *to);
static Node	*call(Token *tok);
static Node	*read_suffix_increment(Node *node);
static Node	*read_deref_index(Node *node);
static Node	*primary(void);
static Node	*arrow_loop(Node *node);
static Node	*arrow(void);
static Node	*unary(void);
static Node	*create_mul(int type, Node *lhs, Node *rhs, Token *tok);
static Node	*mul(void);
static Node	*add(void);
static Node	*shift(void);
static Node	*relational(void);
static Node	*equality(void);
static Node	*bitwise_and(void);
static Node	*bitwise_or(void);
static Node	*bitwise_xor(void);
static Node	*conditional_and(void);
static Node	*conditional_or(void);
static Node	*conditional_op(void);
static Node	*assign(void);
static Node	*expr(void);
static Node	*read_ifblock(void);
static Node	*stmt(void);
static Node	*expect_constant(Type *type);
static void	global_var(Type *type, Token *ident, bool is_extern, bool is_static);
Type	*read_struct_block(Token *ident);
Type	*read_enum_block(Token *ident);
Type	*read_union_block(Token *ident);
static void	funcdef(Type *type, Token *ident, bool is_static);
static void	read_typedef(void);
static void	filescope(void);
void	parse(void);

int		switchCaseCount = 0;
Stack	*sbstack;

// main
extern Token			*g_token;
extern t_deffunc		*g_func_defs[1000];
extern t_deffunc		*g_func_protos[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals[1000];
extern StructDef		*g_struct_defs[1000];
extern EnumDef			*g_enum_defs[1000];
extern UnionDef			*g_union_defs[1000];
extern t_deffunc		*g_func_now;
extern t_linked_list	*g_type_alias;

static int max(int a, int b)
{
	if (a < b)
		return (b);
	return (a);
}

SBData	*sbdata_new(bool isswitch, int start, int end)
{
	SBData	*tmp;

	tmp = (SBData *)calloc(1, sizeof(SBData));
	tmp->isswitch = isswitch;
	tmp->startlabel = start;
	tmp->endlabel = end;

	tmp->type = NULL;
	tmp->cases = NULL;
	tmp->defaultLabel = -1;
	return (tmp);
}

void	sb_forwhile_start(int startlabel, int endlabel)
{
	stack_push(&sbstack, sbdata_new(false, startlabel, endlabel));
}

void	sb_switch_start(Type *type, int endlabel, int defaultLabel)
{
	SBData	*tmp;

	tmp = sbdata_new(true, -1, endlabel);
	tmp->type = type;
	tmp->defaultLabel = defaultLabel;
	stack_push(&sbstack, tmp);
}


SBData	*sb_end(void)
{
	SBData	*result;

	result = stack_pop(&sbstack);
	return (result);
}

SBData	*sb_peek(void)
{
	return (SBData *)stack_peek(sbstack);
}

SBData	*sb_search(bool	isswitch)
{
	Stack	*tmp;
	SBData	*data;

	tmp = sbstack;
	while (tmp != NULL)
	{
		data = (SBData *)tmp->data;
		if (data->isswitch == isswitch)
			return (data);
		tmp = tmp->prev;
	}
	return (NULL);
}

int	add_switchcase(SBData *sbdata, int number)
{
	int			count;
	SwitchCase	*tmp;

	count = switchCaseCount++;

	tmp = (SwitchCase *)malloc(sizeof(SwitchCase));
	tmp->value = number;
	tmp->label = count;
	tmp->next = sbdata->cases;
	sbdata->cases = tmp;

	//TODO 被りチェック

	return (count);
}


t_deffunc	*get_function_by_name(char *name, int len)
{
	int		i;
	t_deffunc	*tmp;

	i = 0;
	while (g_func_defs[i])
	{
		tmp = g_func_defs[i];
		if (tmp->name_len == len && strncmp(tmp->name, name, len) == 0)
			return tmp;
		i++;
	}

	i = 0;
	while (g_func_protos[i])
	{
		tmp = g_func_protos[i];
		if (tmp->name_len == len && strncmp(tmp->name, name, len) == 0)
			return tmp;
		i++;
	}
	return NULL;
}

Node	*new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node *node;

	node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node	*new_node_num(int val)
{
	Node *node;

	node = new_node(ND_NUM, NULL, NULL);
	node->val = val;
	node->type = new_primitive_type(TY_INT);
	return node;
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

// TODO analyze用と分けて、analyze版はanalyze_nodeをしてしまう
Node	*cast(Node *node, Type *to)
{
	node = new_node(ND_CAST, node, NULL);
	node->type = to;
	node->analyze_source = node->lhs->analyze_source;
	return (node);
}

static Node *call(Token *tok)
{
	Node		*node;

	debug("CALL %s", strndup(tok->str, tok->len));

	node							= new_node(ND_CALL, NULL, NULL);
	node->analyze_source			= tok->str;
	node->analyze_funccall_name		= tok->str;
	node->analyze_funccall_name_len	= tok->len;
	node->funccall_argcount			= 0;
	node->funcdef					= get_function_by_name(tok->str, tok->len);

	// read arguments
	for (;;)
	{	
		if (consume(")"))
			break;

		if (node->funccall_argcount != 0 && !consume(","))
			error_at(g_token->str, "トークンが,ではありません");

		node->funccall_args[node->funccall_argcount++] = expr();
	}

	debug(" CALL END");
	return node;
}

/*
 * type:
 * 0 : *
 * 1 : /
 * 2 : %
 */
static Node	*create_mul(int type, Node *lhs, Node *rhs, Token *tok)
{
	Node	*node;

	if (type == 0)
		node = new_node(ND_MUL, lhs, rhs);
	else if (type == 1)
		node = new_node(ND_DIV, lhs, rhs);
	else
		node = new_node(ND_MOD, lhs, rhs);
	node->analyze_source = tok->str;
	return (node);
}

// 後置インクリメント, デクリメント
static Node	*read_suffix_increment(Node *node)
{
	char	*source;

	source = g_token->str;
	if (consume("++"))
	{
		node = new_node(ND_COMP_ADD, node, new_node_num(1));
		node->analyze_source = source;

		node = new_node(ND_SUB, node, new_node_num(1));
		node->analyze_source = source;
	}
	else if (consume("--"))
	{
		node = new_node(ND_COMP_SUB, node, new_node_num(1));
		node->analyze_source = source;

		node = new_node(ND_ADD, node, new_node_num(1));
		node->analyze_source = source;
	}
	return (node);
}

// 添字によるDEREF
// TODO エラーメッセージが足し算用になってしまう
static Node	*read_deref_index(Node *node)
{
	char	*source;

	source = g_token->str;
	while (consume("["))
	{
		node = new_node(ND_ADD, node, expr());
		node->analyze_source = source;

		node = new_node(ND_DEREF, node, NULL);
		node->analyze_source = source;

		if (!consume("]"))
			error_at(g_token->str, "%s");

		source = g_token->str;
	}
	return (read_suffix_increment(node));
}

static Node *primary(void)
{
	Token	*tok;
	Node	*node;
	Type	*type_cast;
	int 	number;

	// 括弧
	if (consume("("))
	{
		// 型を読む
		type_cast = consume_type_before(false);
		
		// 括弧の中身が型ではないなら優先順位を上げる括弧
		if (type_cast == NULL)
		{
			node = new_node(ND_PARENTHESES, expr(), NULL);
			expect(")");
			return read_deref_index(node);
		}

		// 明示的なキャスト
		// TODO キャストの優先順位が違う
		expect(")");
		node = cast(unary(), type_cast);
		return read_deref_index(node);
	}

	// enumの値
	Type	*enum_type;
	int		enum_value;

	if (consume_enum_key(&enum_type, &enum_value))
	{
		node = new_node_num(enum_value);
		node->type = enum_type;
		return read_deref_index(node);
	}

	// 変数かcall
	// TODO 関数を変数として、関数呼び出しをパースする
	tok = consume_ident();
	if (tok)
	{
		// call func
		if (consume("("))
		{
			node = call(tok);
			return read_deref_index(node);
		}

		// 変数
		node						= new_node(ND_ANALYZE_VAR, NULL, NULL);
		node->analyze_source		= tok->str;
		node->analyze_var_name		= tok->str;
		node->analyze_var_name_len	= tok->len;
		return read_deref_index(node);
	}

	// string
	tok = consume_str_literal();
	if (tok)
	{
		node = new_node(ND_STR_LITERAL, NULL, NULL);
		node->def_str = get_str_literal(tok->str, tok->len);
		return read_deref_index(node);
	}

	// char
	tok = consume_char_literal();
	if (tok)
	{
		number = char_to_int(tok->str, tok->strlen_actual);
		if (number == -1)
			error_at(tok->str, "不明なエスケープシーケンスです (primary)");
		node = new_node_num(number);
		node->type = new_primitive_type(TY_CHAR);
		return read_deref_index(node); // charの後ろに[]はおかしいけれど、とりあえず許容
	}

	// 数
	if (!consume_number(&number))
		error_at(g_token->str, "数字が必要です");
	node = new_node_num(number);

	return read_deref_index(node);
}


static Node	*arrow_loop(Node *node)
{
	Token		*ident;
	char		*source;

	source = g_token->str;
	if (consume("->"))
	{
		ident = consume_ident();

		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です\n (arrow_loop)");
	
		node							= new_node(ND_MEMBER_PTR_VALUE, node, NULL);
		node->analyze_source			= source;
		node->analyze_member_name		= ident->str;
		node->analyze_member_name_len	= ident->len;
		node							= read_deref_index(node);
		return arrow_loop(node);
	}
	else if (consume("."))
	{
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です\n (arrow_loop)");

		node							= new_node(ND_MEMBER_VALUE, node, NULL);
		node->analyze_source			= source;
		node->analyze_member_name		= ident->str;
		node->analyze_member_name_len	= ident->len;
		node							= read_deref_index(node);
		return arrow_loop(node);
	}
	else
		return (node);
}

static Node	*arrow(void)
{
	Node	*node;

	node = primary();
	return (arrow_loop(node));
}

static Node *unary(void)
{
	Node	*node;
	Type	*type;
	char	*source;

	source = g_token->str;
	if (consume("+"))
	{
		node = new_node(ND_ADD_UNARY, unary(), NULL);
		node->analyze_source = source;
		return (node);
	}
	else if (consume("-"))
	{
		node = new_node(ND_SUB_UNARY, unary(), NULL);
		node->analyze_source = source;
		return (node);
	}
	else if (consume("*"))
	{
		node = new_node(ND_DEREF, unary(), NULL);
		node->analyze_source = source;
		return (node);
	}
	else if (consume("&"))
	{
		node = new_node(ND_ADDR, unary(), NULL);
		node->analyze_source = source;
		return (node);
	}
	else if (consume("++"))
	{
		node = new_node(ND_COMP_ADD, unary(), new_node_num(1));
		node->analyze_source = source;
		return (node);
	}
	else if (consume("--"))
	{
		node = new_node(ND_COMP_SUB, unary(), new_node_num(1));
		node->analyze_source = source;
		return (node);
	}
	else if (consume("!"))
	{
		node = new_node(ND_EQUAL, unary(), new_node_num(0));
		node->analyze_source = source;
		return (node);
	}
	else if (consume("~"))
	{
		node = new_node(ND_BITWISE_NOT, unary(), NULL);
		node->analyze_source = source;
		return (node);
	}
	else if (consume_with_type(TK_SIZEOF))
	{
		// とりあえずsizeof (型)を読めるようにした
		if (consume("("))
		{
			type = consume_type_before(true);
			if (type == NULL)
				node = new_node(ND_SIZEOF, unary(), NULL);
			else
				node = new_node_num(get_type_size(type));
			if (!consume(")"))
				error_at(g_token->str, ")が必要です");
		}
		else
			node = new_node(ND_SIZEOF, unary(), NULL);
		return (node);
	}

	return arrow();
}

static Node *mul(void)
{
	Node *node;

	node = unary();
	for (;;)
	{
		// TODO sourceが一こ先にずれてる
		if (consume("*"))
			node = create_mul(0, node, unary(), g_token);
		else if (consume("/"))
			node = create_mul(1, node, unary(), g_token);
		else if (consume("%"))
			node = create_mul(2, node, unary(), g_token);
		else
			return (node);
	}
}

static Node	*add(void)
{
	Node	*node;
	char	*source;

	node = mul();
	for (;;)
	{
		source = g_token->str;

		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return (node);

		node->analyze_source = source;
	}
}

static Node	*shift(void)
{
	Node	*node;
	char	*source;

	node = add();
	source = g_token->str;
	if (consume("<<"))
	{
		node = new_node(ND_SHIFT_LEFT, node, shift());
		node->analyze_source = source;
	}
	else if (consume(">>"))
	{
		node = new_node(ND_SHIFT_RIGHT, node, shift());
		node->analyze_source = source;
	}
	return (node);
}

static Node	*relational(void)
{
	Node	*node;
	char	*source;

	node = shift();
	for (;;)
	{
		source = g_token->str;

		if (consume("<"))
			node = new_node(ND_LESS, node, shift());
		else if (consume("<="))
			node = new_node(ND_LESSEQ, node, shift());
		else if (consume(">"))
			node = new_node(ND_LESS, shift(), node);
		else if (consume(">="))
			node = new_node(ND_LESSEQ, shift(), node);
		else
			return (node);

		node->analyze_source = source;
	}
}

static Node *equality(void)
{
	Node	*node;
	char	*source;

	node = relational();
	for (;;)
	{
		source = g_token->str;

		if (consume("=="))
			node = new_node(ND_EQUAL, node, relational());
		else if (consume("!="))
			node = new_node(ND_NEQUAL, node, relational());
		else
			return (node);

		node->analyze_source = source;
	}
}

static Node	*bitwise_and(void)
{
	Node	*node;
	char	*source;

	node = equality();
	source = g_token->str;
	if (consume("&"))
	{
		node = new_node(ND_BITWISE_AND, node, bitwise_and());
		node->analyze_source = source;
	}
	return (node);
}


static Node	*bitwise_xor(void)
{
	Node	*node;
	char	*source;

	node = bitwise_and();
	source = g_token->str;
	if (consume("^"))
	{
		node = new_node(ND_BITWISE_XOR, node, bitwise_xor());
		node->analyze_source = source;
	}
	return (node);
}

static Node	*bitwise_or(void)
{
	Node	*node;
	char	*source;

	node = bitwise_xor();
	source = g_token->str;
	if (consume("|"))
	{
		node = new_node(ND_BITWISE_OR, node, bitwise_or());
		node->analyze_source = source;
	}
	return (node);
}

static Node	*conditional_and(void)
{
	Node	*node;

	node = bitwise_or();
	if (consume("&&"))
		node = new_node(ND_COND_AND, node, conditional_and());
	return (node);
}

static Node	*conditional_or(void)
{
	Node	*node;

	node = conditional_and();
	if (consume("||"))
		node = new_node(ND_COND_OR, node, conditional_or());
	return (node);
}

static Node	*conditional_op(void)
{
	Node	*node;
	char	*source;

	source = g_token->str;
	node = conditional_or();
	if (consume("?"))
	{
		node = new_node(ND_COND_OP, node, conditional_op());
		node->analyze_source = source;

		if (!consume(":"))
			error_at(g_token->str, ":が必要です");

		node->els = conditional_op();
	}
	return (node);
}

static Node	*assign(void)
{
	Node	*node;
	char	*source;

	node = conditional_op();

	source = g_token->str;
	if (consume("="))
	{
		node = new_node(ND_ASSIGN, node, assign());
		node->analyze_source = source;
	}
	else if (consume("+="))
	{
		node = new_node(ND_COMP_ADD, node, assign());
		node->analyze_source = source;
	}
	else if (consume("-="))
	{
		node = new_node(ND_COMP_SUB, node, assign());
		node->analyze_source = source;
	}
	else if (consume("*="))
	{
		node = create_mul(0, node, assign(), g_token);
		node->kind = ND_COMP_MUL;
	}
	else if (consume("/="))
	{
		node = create_mul(1, node, assign(), g_token);
		node->kind = ND_COMP_DIV;
	}
	else if (consume("%="))
	{
		node = create_mul(2, node, assign(), g_token);
		node->kind = ND_COMP_MOD;
	}
	return (node);
}

static Node	*expr(void)
{
	return assign();
}

// ifの後ろの括弧から読む
static Node	*read_ifblock(void)
{
	Node	*node;

	debug("  IF START");

	if (!consume("("))
		error_at(g_token->str, "(ではないトークンです");
	node = new_node(ND_IF, expr(), NULL);

	debug("   IF READed EXPR");

	if (!consume(")"))
		error_at(g_token->str, ")ではないトークンです");
	node->rhs = stmt();

	debug("   IF READ STMT");

	if (consume_with_type(TK_ELSE))
	{
		if (consume_with_type(TK_IF))
			node->elsif = read_ifblock();
		else
			node->els = stmt();
	}
	debug("  IF END");
	return (node);
}

// TODO 条件の中身がintegerか確認する
static Node	*stmt(void)
{
	Node	*node;
	Type	*type;
	Token 	*ident;
	int		number;
	Node	*start;
	char	*source;

	source = g_token->str;
	if (consume_with_type(TK_RETURN))
	{
		if (consume(";"))
		{
			node = new_node(ND_RETURN, NULL, NULL);
		}
		else
		{
			node = new_node(ND_RETURN, expr(), NULL);
			expect_semicolon();
		}
		node->analyze_source = source;
		return (node);
	}
	else if (consume_with_type(TK_IF))
	{
		node = read_ifblock();
		node->analyze_source = source;
		return (node);
	}
	else if (consume_with_type(TK_WHILE))
	{
		debug("  WHILE START");

		if (!consume("("))
			error_at(g_token->str, "(ではないトークンです");

		node = new_node(ND_WHILE, expr(), NULL);
		node->analyze_source = source;

		if (!consume(")"))
			error_at(g_token->str, ")ではないトークンです");

		if (!consume(";"))
			node->rhs = stmt();

		debug("  WHILE END");
		return node;
	}
	else if (consume_with_type(TK_DO))
	{
		node = new_node(ND_DOWHILE, stmt(), NULL);
		node->analyze_source = source;

		if (!consume_with_type(TK_WHILE))
			error_at(g_token->str, "whileが必要です");

		if (!consume("("))
			error_at(g_token->str, "(ではないトークンです");
		if (!consume(";"))
			node->rhs = expr();
		if (!consume(")"))
			error_at(g_token->str, ")ではないトークンです");
	}
	else if (consume_with_type(TK_FOR))
	{
		debug("  FOR START");

		if (!consume("("))
			error_at(g_token->str, "(ではないトークンです");

		node = new_node(ND_FOR, NULL, NULL);
		node->analyze_source = source;

		// for init
		if (!consume(";"))
		{
			node->for_expr[0] = expr();
			expect_semicolon();
		}
		// for if
		if (!consume(";"))
		{
			node->for_expr[1] = expr();
			expect_semicolon();
		}
		// for next
		if (!consume(")"))
		{
			node->for_expr[2] = expr();
			if(!consume(")"))
				error_at(g_token->str, ")ではないトークンです");
		}

		// stmt
		if (!consume(";"))
			node->lhs = stmt();

		debug("  FOR END");
		return (node);
	}
	else if (consume_with_type(TK_SWITCH))
	{
		if (!consume("("))
			error_at(g_token->str, "(ではないトークンです");

		node = new_node(ND_SWITCH, expr(), NULL);
		node->analyze_source = source;

		if (!consume(")"))
			error_at(g_token->str, ")ではないトークンです");

		node->rhs = stmt();
		return (node);
	}
	else if (consume_with_type(TK_CASE))
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

		node = new_node(ND_CASE, NULL, NULL);
		node->analyze_source = source;
		node->val = number;
		return (node);
	}
	else if (consume_with_type(TK_BREAK))
	{
		expect_semicolon();

		node = new_node(ND_BREAK, NULL, NULL);
		node->analyze_source = source;
		return (node);
	}
	else if (consume_with_type(TK_CONTINUE))
	{
		expect_semicolon();

		node = new_node(ND_CONTINUE, NULL, NULL);
		node->analyze_source = source;
		return (node);
	}
	else if (consume_with_type(TK_DEFAULT))
	{
		if (!consume(":"))
			error_at(g_token->str, ":が必要です");
		node = new_node(ND_DEFAULT, NULL, NULL);
		node->analyze_source = source;
		return (node);
	}
	else if (consume("{"))
	{
		node = new_node(ND_BLOCK, NULL, NULL);
		node->analyze_source = source;

		start = node;

		while (!consume("}"))
		{
			debug(" START READ BLOCK LINE");
			node->lhs = stmt();
			node->rhs = new_node(ND_BLOCK, NULL, NULL);
			node = node->rhs;
			debug(" END READ BLOCK LINE");
		}
		return (start);
	}
	else
	{
		// 構造体なら宣言の可能性がある
		// TODO ここはfilescopeのコピペ
		if (consume_with_type(TK_STRUCT))
		{
			ident = consume_ident();
			if (ident == NULL)
				error_at(g_token->str, "構造体の識別子が必要です");

			if (consume("{"))
			{
				type = read_struct_block(ident);
				// ;なら構造体の宣言
				if (consume(";"))
					return new_node(ND_NONE, NULL, NULL);
			}
			else
				type = new_struct_type(ident->str, ident->len);
			consume_type_ptr(&type);
		}
		// enumの可能性
		else if (consume_with_type(TK_ENUM))
		{
			ident = consume_ident();
			if (ident == NULL)
				error_at(g_token->str, "enumの識別子が必要です");

			if (consume("{"))
			{
				type = read_enum_block(ident);
				// ;ならenumの宣言
				if (consume(";"))
					return new_node(ND_NONE, NULL, NULL);
			}
			else
				type = new_enum_type(ident->str, ident->len);
			consume_type_ptr(&type);
		}
		// unionの可能性
		else if (consume_with_type(TK_UNION))
		{
			ident = consume_ident();
			if (ident == NULL)
				error_at(g_token->str, "unionの識別子が必要です");

			if (consume("{"))
			{
				type = read_union_block(ident);
				// ;ならunionの宣言
				if (consume(";"))
					return (new_node(ND_NONE, NULL, NULL));
			}
			else
				type = new_union_type(ident->str, ident->len);
			consume_type_ptr(&type);
		}
		else
			type = consume_type_before(false);

		if (type != NULL)
		{
			ident = consume_ident();

			if (ident == NULL)
				error_at(g_token->str, "識別子が必要です");

			expect_type_after(&type);

			node = new_node(ND_VAR_DEF, NULL, NULL);
			node->type = type;
			node->analyze_source = ident->str;
			node->analyze_var_name = ident->str;
			node->analyze_var_name_len = ident->len;

			// 宣言と同時に代入
			if (consume("="))
				node->lvar_assign = expr();
		}
		else
		{
			node = expr();
		}
	}
	if(!consume(";"))
		error_at(g_token->str, ";ではないトークン(Kind : %d , %s)です", g_token->kind, strndup(g_token->str, g_token->len));

	return node;
}

static Node	*expect_constant(Type *type)
{
	Node	*node;
	Node	**next;
	int		number;
	Token	*tok;

	if (is_integer_type(type) && consume_number(&number))
	{
		node = new_node_num(number);
	}
	else if (type_equal(type, new_type_ptr_to(new_primitive_type(TY_CHAR)))
			&& (tok = consume_str_literal()) != NULL)
	{
		node = new_node(ND_STR_LITERAL, NULL, NULL);
		node->def_str = get_str_literal(tok->str, tok->len);
	}
	else if (type_equal(type, new_primitive_type(TY_CHAR)) 
			&& (tok = consume_char_literal()) != NULL)
	{
		number = char_to_int(tok->str, tok->strlen_actual);
		if (number == -1)
			error_at(tok->str, "不明なエスケープシーケンスです (expect_constant)");
		node = new_node_num(number);
	}
	else if (is_pointer_type(type) && consume("{"))
	{
		node = NULL;
		next = &node;
		for (;;)
		{
			*next = expect_constant(type->ptr_to);
			next = &(*next)->global_assign_next;
			if (consume("}"))
				break ;
			if (!consume(","))
				error_at(g_token->str, ",が必要です");
		}
	}
	else
	{
		node = NULL;
		error_at(g_token->str, "定数が必要です");
	}
	return (node);
}

static void	global_var(Type *type, Token *ident, bool is_extern, bool is_static)
{
	int			i;
	t_defvar	*defvar;

	// 後ろの型を読む
	// TODO a[]とかでも許容したい
	expect_type_after(&type);

	defvar				= calloc(1, sizeof(t_defvar));
	defvar->name		= ident->str;
	defvar->name_len	= ident->len;
	defvar->type		= type;
	defvar->is_extern	= is_extern;
	defvar->is_static	= is_static;

	// TODO チェックは違うパスでやりたい....
	if (is_static && is_extern)
		error_at(g_token->str, "staticとexternは併用できません");
	if (!is_declarable_type(type))
		error_at(g_token->str, "宣言できない型の変数です");

	// 保存
	i = -1;
	while (g_global_vars[++i]);
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
Type	*read_struct_block(Token *ident)
{
	Type		*type;
	StructDef	*def;
	MemberElem	*tmp;
	int			typesize;
	int			maxsize;
	int			i;

	def = calloc(1, sizeof(StructDef));
	def->name = ident->str;
	def->name_len = ident->len;
	def->mem_size = -1;
	def->members = NULL;

	// 保存
	for (i = 0; g_struct_defs[i]; i++)
		continue ;
	g_struct_defs[i] = def;

	debug(" READ STRUCT %s", strndup(ident->str, ident->len));

	while (1)
	{
		if (consume("}"))
			break;

		type = consume_type_before(true);
		if (type == NULL)
			error_at(g_token->str, "型宣言が必要です\n (read_struct_block)");
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です\n (read_struct_block)");
		expect_type_after(&type);

		expect_semicolon();

		tmp = calloc(1, sizeof(MemberElem));
		tmp->name = ident->str;
		tmp->name_len = ident->len;
		tmp->type = type;
		tmp->next = def->members;

		// 型のサイズを取得
		typesize = get_type_size(type);
		if (typesize == -1)
			error_at(ident->str, "型のサイズが確定していません");

		maxsize = max_type_size(type);

		// offsetをoffset + typesizeに設定
		if (def->members ==  NULL)
			tmp->offset = typesize;
		else
		{
			i = def->members->offset;
			if (maxsize < 4)
			{
				if (i % 4 + typesize > 4)
					tmp->offset = ((i + 4) / 4 * 4) + typesize;
				else
					tmp->offset = i + typesize;
			}
			else if (maxsize == 4)
				tmp->offset = ((i + 3) / 4) * 4 + typesize;
			else
				tmp->offset = ((i + 7) / 8) * 8 + typesize;
		}

		debug("  OFFSET OF %s : %d", strndup(ident->str, ident->len), tmp->offset);
		def->members = tmp;
	}

	type = new_struct_type(def->name, def->name_len);

	// メモリサイズを決定
	if (def->members == NULL)
		def->mem_size = 0;
	else
	{
		maxsize = max_type_size(type);
		debug("  MAX_SIZE = %d", maxsize);
		def->mem_size = align_to(def->members->offset, maxsize);
	}
	debug("  MEMSIZE = %d", def->mem_size);

	// offsetを修正
	for (tmp = def->members; tmp != NULL; tmp = tmp->next)
	{
		tmp->offset -= get_type_size(tmp->type);
	}

	return (type);
}

// {以降を読む
Type	*read_enum_block(Token *ident)
{
	int		i;
	EnumDef	*def;
	Type	*type;

	def = calloc(1, sizeof(EnumDef));
	def->name = ident->str;
	def->name_len = ident->len;
	def->kind_len = 0;

	// 保存
	for (i = 0; g_enum_defs[i]; i++)
		continue ;
	g_enum_defs[i] = def;

	debug(" READ ENUM %s", strndup(ident->str, ident->len));

	while (1)
	{
		if (consume("}"))
			break ;
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が見つかりません");
		def->kinds[def->kind_len++] = strndup(ident->str, ident->len);
		if (!consume(","))
		{
			if (!consume("}"))
				error_at(g_token->str, "}が見つかりません");
			break ;
		}
		debug(" ENUM %s", def->kinds[def->kind_len - 1]);
	}

	type = new_enum_type(def->name, def->name_len);
	return (type);
}

// {以降を読む
Type	*read_union_block(Token *ident)
{
	UnionDef	*def;
	MemberElem	*tmp;
	Type		*type;
	int			i;
	int			typesize;

	def = calloc(1, sizeof(UnionDef));
	def->name = ident->str;
	def->name_len = ident->len;
	def->mem_size = 0;
	def->members = NULL;

	// 保存
	for (i = 0; g_union_defs[i]; i++)
		continue ;
	g_union_defs[i] = def;

	debug(" READ UNION %s", strndup(ident->str, ident->len));

	// 要素を追加 & 最大のサイズを取得
	while (1)
	{
		if (consume("}"))
			break;

		type = consume_type_before(true);
		if (type == NULL)
			error_at(g_token->str, "型宣言が必要です\n (read_union_block)");
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です\n (read_union_block)");
		expect_type_after(&type);

		expect_semicolon();

		tmp = calloc(1, sizeof(MemberElem));
		tmp->name = ident->str;
		tmp->name_len = ident->len;
		tmp->type = type;
		tmp->next = def->members;
		tmp->offset = 0;

		// 型のサイズを取得
		typesize = get_type_size(type);
		if (typesize == -1)
			error_at(ident->str, "型のサイズが確定していません");
		def->mem_size = max(def->mem_size, typesize);

		def->members = tmp;
	}

	debug("  MEMSIZE = %d", def->mem_size);

	type = new_union_type(def->name, def->name_len);
	return (type);
}


// TODO ブロックを抜けたらlocalsを戻す
// TODO 変数名の被りチェックは別のパスで行う
// (まで読んだところから読む
static void	funcdef(Type *type, Token *ident, bool is_static)
{
	t_deffunc	*def;
	Token		*arg;
	int			i;

	def							= calloc(1, sizeof(t_deffunc));
	def->name					= ident->str;
	def->name_len				= ident->len;
	def->type_return			= type;
	def->argcount				= 0;
	def->is_static				= is_static;
	g_func_now = def;

	// args
	if (!consume(")"))
	{
		for (;;)
		{
			// variable argument
			if (consume("..."))
			{
				if (def->type_arguments[0] == NULL)
					error_at(g_token->str, "可変長引数の宣言をするには、少なくとも一つの引数が必要です");
				if (!consume(")"))
					error_at(g_token->str, ")が必要です");
				def->is_variable_argument = true;
				break ;
			}

			// 型宣言の確認
			type = consume_type_before(false);
			if (type == NULL)
				error_at(g_token->str,"型が必要です\n (funcdef)");

			// 仮引数名
			arg = consume_ident();
			if (arg == NULL)
			{
				// voidなら引数0個
				if (type->ty == TY_VOID)
				{
					if (def->type_arguments[0] != NULL)
						error_at(g_token->str, "既に引数が宣言されています");
					if (!consume(")"))
						error_at(g_token->str, ")が見つかりませんでした。");
					def->is_zero_argument = true;
					break ;
				}
				error_at(g_token->str, "仮引数が必要です");
			}

			// arrayを読む
			expect_type_after(&type);

			// save
			def->argument_names[def->argcount] = arg->str;
			def->argument_name_lens[def->argcount] = arg->len;
			def->type_arguments[def->argcount] = type;
			def->argcount += 1;

			// )か,
			if (consume(")"))
				break;
			if (!consume(","))
				error_at(g_token->str, ",が必要です");
		}
	}

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

	g_func_now = NULL;
	
	debug(" CREATED FUNC %s", strndup(def->name, def->name_len));
}

static void	read_typedef(void)
{
	Type		*type;
	Token		*token;
	TypedefPair	*pair;

	// 型を読む
	type = consume_type_before(true);
	if (type == NULL)
		error_at(g_token->str, "型宣言が必要です\n (read_typedef)");
	expect_type_after(&type);

	// 識別子を読む
	token = consume_ident();
	if (token == NULL)
		error_at(g_token->str, "識別子が必要です");

	// ペアを追加
	pair = malloc(sizeof(TypedefPair));
	pair->name = token->str;
	pair->name_len = token->len;
	pair->type = type;
	linked_list_insert(g_type_alias, pair);

	expect_semicolon();
}

static void	filescope(void)
{
	Token	*ident;
	Type	*type;
	bool	is_static;
	bool	is_inline;

	// typedef
	if (consume_with_type(TK_TYPEDEF))
	{
		read_typedef();
		return ;
	}

	// extern
	if (consume_with_type(TK_EXTERN))
	{
		type = consume_type_before(true);
		if (type == NULL)
			error_at(g_token->str, "型が必要です");
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です");
		global_var(type, ident, true, false);
		return ;
	}

	is_static = false;
	if (consume_with_type(TK_STATIC))
	{
		is_static = true;
	}

	// TODO とりあえず無視
	is_inline = false;
	if (consume_with_type(TK_INLINE))
	{
		is_inline = true;
	}

	// structの宣言か返り値がstructか
	if (consume_with_type(TK_STRUCT))
	{
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です");
			
		if (consume("{"))
		{
			read_struct_block(ident);
			// ;なら構造体の宣言
			// そうでないなら返り値かグローバル変数
			if (consume(";"))
				return ;
		}
		type = new_struct_type(ident->str, ident->len);
		consume_type_ptr(&type);
	}
	// enumの宣言か返り値がenumか
	else if (consume_with_type(TK_ENUM))
	{
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です");
			
		if (consume("{"))
		{
			read_enum_block(ident);
			// ;ならenumの宣言
			// そうでないなら返り値かグローバル変数
			if (consume(";"))
				return ;
		}
		type = new_enum_type(ident->str, ident->len);
		consume_type_ptr(&type);
	}
	// unionの宣言か返り値がunionか
	else if (consume_with_type(TK_UNION))
	{
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "識別子が必要です");
			
		if (consume("{"))
		{
			read_union_block(ident);
			// ;ならunionの宣言
			// そうでないなら返り値かグローバル変数
			if (consume(";"))
				return ;
		}
		type = new_union_type(ident->str, ident->len);
		consume_type_ptr(&type);
	}
	else
		type = consume_type_before(true);

	// TODO 一旦staticは無視
	// グローバル変数か関数宣言か
	if (type != NULL)
	{
		// ident
		ident = consume_ident();
		if (ident == NULL)
			error_at(g_token->str, "不明なトークンです");

		// function definition
		if (consume("("))
			funcdef(type, ident, is_static);
		else
			global_var(type, ident, false, is_static);
		return ;
	}

	error_at(g_token->str, "構文解析に失敗しました[filescope kind:%d]", g_token->kind);
}

void	parse(void)
{
	while (!at_eof())
		filescope();
}
