#ifndef STDARG_H
# define STDARG_H

// TODO unsigned 
typedef struct s_va_list{
	int gp_offset;
	int fp_offset;
	void *overflow_arg_area;
	void *reg_save_area;
} va_list;

void va_start(va_list ap);
void va_arg(va_list ap);
void va_end(va_list ap);

#endif
