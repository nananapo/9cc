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
void	consume_type_ptr(t_type **type);
t_type	*consume_type_before(int read_def);
void	expect_type_after(t_type **type);
void	expect(char *op);
int		expect_number(void);

void	expect_semicolon(void);
bool	at_eof(void);
#endif
