#ifndef PARSE_H
# define PARSE_H

# include "9cc.h"

bool	consume(char *op);
bool	consume_number(int *result);
bool	consume_with_type(TokenKind kind);
Token	*consume_ident(void);
Token	*consume_ident_str(char *p);
Token	*consume_str_literal(void);
Token	*consume_char_literal(void);
void	consume_type_ptr(Type **type);
Type	*consume_type_before(int read_def);
void	expect_type_after(Type **type);
void	expect(char *op);
int		expect_number(void);
bool	at_eof(void);

t_defvar	*find_global(char *str, int len);

#endif
