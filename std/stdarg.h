#ifndef STDARG_H
# define STDARG_H

// TODO unsigned 
typedef struct va_list{
	int gp_offset;
	int fp_offset;
	void *overflow_arg_area;
	void *reg_save_area;
} va_list;

void va_start(va_list ap, char *fmt);

#endif
