#ifndef PARSE_H
# define PARSE_H

# include "9cc.h"

bool	consume(ParseResult *env, char *op);
bool	consume_number(ParseResult *env, int *result);
bool	consume_with_type(ParseResult *env, TokenKind kind);
Token	*consume_ident(ParseResult *env);
Token	*consume_ident_str(ParseResult *env, char *p);
Token	*consume_str_literal(ParseResult *env);
Token	*consume_char_literal(ParseResult *env);
void	consume_type_ptr(ParseResult *env, Type **type);
Type	*consume_type_before(ParseResult *env);
void	expect_type_after(ParseResult *env, Type **type);
void	expect(ParseResult *env, char *op);
int		expect_number(ParseResult *env);
bool	at_eof(ParseResult *env);

Node	*find_global(ParseResult *env, char *str, int len);

#endif
