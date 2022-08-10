#ifndef STDARG_H
# define STDARG_H

/* x86_64
// TODO unsigned 
typedef struct va_list{
	int gp_offset;
	int fp_offset;
	void *overflow_arg_area;
	void *reg_save_area;
}[1] va_list;
*/

// riscv
#define va_list void *

void va_start(va_list ap, ...);

#endif
