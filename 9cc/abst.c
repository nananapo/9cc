#include "9cc.h"

extern t_arch	g_arch;

void    codegen_x8664(void);
void    codegen_riscv(void);

int	get_array_align_size_x8664(t_type *type);
int	get_type_size_x8664(t_type *type);

int	get_array_align_size_riscv(t_type *type);
int	get_type_size_riscv(t_type *type);

int	get_type_size(t_type *type)
{
	if (g_arch == ARCH_X8664)
		return (get_type_size_x8664(type));
	else if (g_arch == ARCH_RISCV)
		return (get_type_size_riscv(type));
	return (-1);
}

int	get_array_align_size(t_type *type)
{
	if (g_arch == ARCH_X8664)
		return (get_array_align_size_x8664(type));
	else if (g_arch == ARCH_RISCV)
		return (get_array_align_size_riscv(type));
	return (-1);
}

void	codegen(void)
{
	if (g_arch == ARCH_X8664)
		codegen_x8664();
	else if (g_arch == ARCH_RISCV)
		codegen_riscv();
}
