#ifndef PARSE_H
# define PARSE_H

# include "9cc.h"

bool	consume(char *op);
bool	consume_number(int *result);
bool	consume_kind(t_tokenkind kind);
t_token	*consume_ident(void);
t_token	*consume_ident_str(char *p);
t_token	*consume_str_literal(void);
t_token	*consume_char_literal(void);
bool	consume_type(t_type **type, t_token **ident, bool *is_static, bool *is_extern);
void	expect(char *op);
int		expect_number(void);

void	expect_semicolon(void);
bool	at_eof(void);

/*
typedef enum e_storage_class
{
	SC_NONE,
	SC_AUTO,
	SC_REGISTER,
	SC_STATIC,
	SC_EXTERN,
	SC_TYPEDEF
}	t_storage_class;

typedef enum e_type_specifier_kind
{
	TS_VOID,
	TS_CHAR,
	TS_SHORT,
	TS_INT,
	TS_LONG,
	TS_FLOAT,
	TS_DOUBLE,
	TS_SIGNED,
	TS_UNSIGNED,
	TS_STRUCT,
	TS_UNION,
	TS_ENUM,
	TS_TYPEDEF_NAME
}	t_type_specifier_kind;

typedef struct s_function_definition t_function_definition;
typedef struct s_declaration_specifiers t_declaration_specifiers;
typedef struct s_declarator	t_declarator;
typedef struct s_declaration_list t_declaration_list;
typedef struct s_compound_statement t_compound_statement;
typedef struct s_type_speciifier t_type_specifier;

struct s_function_definition
{
	t_declaration_specifiers	*declaration_specifiers;
	t_declarator				*declarator;
	t_declaration_list			*declaration_list;
	t_compound_statement		*compound_statement;
};

struct s_type_specifier
{
	t_type_specifiers_kind	kind;
}
*/

#endif
