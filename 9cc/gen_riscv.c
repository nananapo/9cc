#include "9cc.h"
#include "il.h"
#include "charutil.h"
#include "mymath.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

// スタックは下に伸びる
// 16 byteアライン
// riscv-64

static void	push(void);
static void	pushi(int data);
static void	pop(char *reg);
static void	mov(char *dst, char *from);
static void	movi(char *dst, int i);
static void	cmps(char *dst, char *from);
static void	cmp(t_type *type);
static void	store_value(int size);
static void	store_ptr(int size, bool minus_step);
static void	load(t_type *type);
static void	cast(t_type *from, t_type *to);
static void	print_global_constant(t_node *node, t_type *type);

static char		*arg_regs[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static int		stack_count = 0;
static int		aligned_stack_def_var_end;
static int		locals_stack_add = 0;

static int		g_locals_count;
static t_lvar	*g_locals[1000];
static int		g_locals_offset[1000];
static int		g_locals_regindex[1000];

static int		g_call_locals_count;
static t_lvar	*g_call_locals[1000];
static int		g_call_locals_offset[1000];
static int		g_call_locals_regindex[1000];

static int		g_call_memory_count;
static t_il		*g_call_memory_codes[1000];
static t_lvar	*g_call_memory_lvars[1000];


// main
extern t_deffunc		*g_func_defs[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals[1000];
extern t_il				*g_il;



static int	max_align_size(t_type *type)
{
	t_member	*tmp;
	int			size;

	if (type->ty == TY_STRUCT)
	{
		size = 0;
		for (tmp = type->strct->members; tmp; tmp = tmp->next)
		{
			size = max(size, max_align_size(tmp->type));
		}
		return (size);
	}
	else if (type->ty == TY_ARRAY)
		return (max_align_size(type->ptr_to));
	return (get_type_size(type));
}

static int	get_member_offset(t_member *mem)
{
	t_member	*tmp;
	t_member	*last;
	int			offset;
	int			typesize;
	int			maxsize;

	if (!mem->is_struct)
		return (0);

	offset	= 0;
	last	= NULL;
	for (tmp = mem->strct->members; last != mem && tmp != NULL; tmp = tmp->next)
	{
		typesize = get_type_size(tmp->type);
		if (offset == 0)
		{
			offset = typesize;
		}	
		else
		{
			maxsize = max_align_size(tmp->type);
			if (maxsize < 4)
			{
				if (offset % 4 + typesize > 4)
					offset = ((offset + 4) / 4 * 4) + typesize;
				else
					offset = offset + typesize;
			}
			else if (maxsize == 4)
				offset = ((offset + 3) / 4) * 4 + typesize;
			else
				offset = ((offset + 7) / 8) * 8 + typesize;
		}
		last = tmp;
	}

	offset -= get_type_size(mem->type);
	return (offset);
}

// TODO voidのarray
static int	get_struct_size(t_type *type)
{
	t_defstruct	*def;
	t_member	*mem;

	def			= type->strct;
	if (def->members == NULL)
		return (0);

	for (mem = def->members; mem->next != NULL; mem = mem->next) ;
	return (align_to(get_member_offset(mem) + get_type_size(mem->type), max_align_size(type)));
}

static int	get_union_size(t_defunion *def)
{
	int			tmp_size;
	t_member	*mem;

	tmp_size = 0;
	for (mem = def->members; mem != NULL; mem = mem->next)
		tmp_size = max(tmp_size, get_type_size(mem->type));
	return (tmp_size);
}

int	get_array_align_size_riscv(t_type *type)
{
	return (get_type_size(type));
}

int	get_type_size_riscv(t_type *type)
{
	switch (type->ty)
	{
		case TY_INT:
			return (4);
		case TY_CHAR:
			return (1);
		case TY_BOOL:
			return (1);
		case TY_PTR:
			return (8);
		case TY_ARRAY:
			return (get_type_size(type->ptr_to) * type->array_size);
		case TY_VOID:
			return (1);
		case TY_ENUM:
			return  (4);
		case TY_STRUCT:
			return (get_struct_size(type));
		case TY_UNION:
			return (get_union_size(type->unon));
		default:
			error("size of unknown type %d", type->ty);
	}
	return (-1);
}

static int	alloc_argument(t_lvar *lvar)
{
	int		i;
	int		size;
	int		regindex_max;
	int		offset_min;
	int		offset_max;
	t_lvar	*var_loop;
	int		offset_loop;
	int		reg_loop;

	// rbpをプッシュした分を考慮する
	offset_min = -16;
	offset_max = 0;
	regindex_max = -1;

	for (i = 0; i < g_locals_count; i++)
	{
		var_loop	= g_locals[i];
		offset_loop	= g_locals_offset[i];
		reg_loop	= g_locals_regindex[i];
		if (var_loop->is_argument)
		{
			regindex_max = max(regindex_max, reg_loop);
			if (reg_loop == -1)
			{
				offset_min = min(offset_min, offset_loop -
								align_to(get_type_size(var_loop->type), 8));
			}
			offset_max = max(offset_max, max(0, offset_loop));
		}
	}

	size = get_type_size(type_array_to_ptr(lvar->type));

	// レジスタに入れる
	if (regindex_max < ARGREG_SIZE - 1
	&& (ARGREG_SIZE - regindex_max - 1) * 8 >= size)
	{
		g_locals[g_locals_count]			= lvar;
		g_locals_regindex[g_locals_count]	= regindex_max + align_to(size, 8) / 8;
		g_locals_offset[g_locals_count]		= (offset_max + size + 7) / 8 * 8;
	}
	// スタックに入れる
	else
	{
		g_locals[g_locals_count]			= lvar;
		g_locals_regindex[g_locals_count]	= -1;
		g_locals_offset[g_locals_count]		= offset_min;
	}

	g_locals_count += 1;

	return (g_locals_count - 1);
}

static void	append_local(t_lvar *def)
{
	int	i;
	int	size;
	int	regindex;
	int	offset;
	int	old;

	size = get_type_size(def->type);

	if (def->is_argument)
	{
		i = alloc_argument(def);

		size	= align_to(size, 8); // 8byte
		regindex= g_locals_regindex[i];
		offset	= g_locals_offset[i];

		if (regindex == -1)
			return ;
		
		// スタックに領域確保
		printf("    sub rsp, %d # %d\n", size, locals_stack_add);
		stack_count			+= size;
		locals_stack_add	+= size;

		mov(R10, RBP);
		printf("    sub %s, %d\n", R10, offset);

		while (size >= 8)
		{
			mov(RAX, arg_regs[regindex--]);
			store_value(8);
			size -= 8;
			if (size != 0)
				printf("    add %s, 8\n", R10);
		}
		return ;
	}

	offset = 0;
	for (i = 0; i < g_locals_count; i++)
	{
		offset = max(g_locals_offset[i], offset);
	}

	g_locals[g_locals_count]			= def;
	g_locals_offset[g_locals_count]		= align_to(offset + size, 8);
	g_locals_regindex[g_locals_count]	= -1;

	g_locals_count += 1;

	printf("    sub %s, %d\n", RSP, align_to(offset + size, 8) - offset);

	old				= stack_count;
	stack_count		+= align_to(offset + size, 8) - offset;
	locals_stack_add+= stack_count - old;
}

static int	append_argument_call(t_lvar *lvar)
{
	int		i;
	int		size;
	int		regindex_max;
	int		offset_min;
	int		offset_max;
	t_lvar	*var_loop;
	int		offset_loop;
	int		reg_loop;

	// rbpをプッシュした分を考慮する
	offset_min = -16;
	offset_max = 0;
	regindex_max = -1;

	for (i = 0; i < g_call_locals_count; i++)
	{
		var_loop	= g_call_locals[i];
		offset_loop	= g_call_locals_offset[i];
		reg_loop	= g_call_locals_regindex[i];
		if (var_loop->is_argument)
		{
			regindex_max = max(regindex_max, reg_loop);
			if (reg_loop == -1)
			{
				offset_min = min(offset_min, offset_loop -
								align_to(get_type_size(var_loop->type), 8));
			}
			offset_max = max(offset_max, max(0, offset_loop));
		}
	}

	size = get_type_size(type_array_to_ptr(lvar->type));

	// レジスタに入れる
	if (regindex_max < ARGREG_SIZE - 1
	&& (ARGREG_SIZE - regindex_max - 1) * 8 >= size)
	{
		g_call_locals[g_call_locals_count]			= lvar;
		g_call_locals_regindex[g_call_locals_count]	= regindex_max + align_to(size, 8) / 8;
		g_call_locals_offset[g_call_locals_count]	= (offset_max + size + 7) / 8 * 8;
	}
	// スタックに入れる
	else
	{
		g_call_locals[g_call_locals_count]			= lvar;
		g_call_locals_regindex[g_call_locals_count]	= -1;
		g_call_locals_offset[g_call_locals_count]	= offset_min;
	}

	g_call_locals_count += 1;

	return (g_call_locals_count - 1);
}

static void	set_call_memory(t_il *code, t_lvar *lvar)
{
	g_call_memory_codes[g_call_memory_count]	= code;
	g_call_memory_lvars[g_call_memory_count]	= lvar;
	g_call_memory_count += 1;
}

// 返り値がmemoryな関数を読んだ時の保存場所を返す
static int	get_call_memory(t_il *code)
{
	int	i;
	int	j;

	for (i = 0; i < g_call_memory_count; i++)
	{
		if (g_call_memory_codes[i] == code)
		{
			for (j = 0; j < g_locals_count; j++)
			{
				if (g_call_memory_lvars[i] == g_locals[j])
					return (g_locals_offset[j]);
			}
			return (-1);
		}
	}
	return (-1);
}



static void	push()
{
	stack_count += 8;
	printf("    %s %s # %d -> %d\n", ASM_PUSH, RAX, stack_count - 8, stack_count);
}

static void	pushi(int num)
{
	stack_count += 8;
	printf("    %s %d # %d -> %d\n", ASM_PUSH, num, stack_count - 8, stack_count);
}

static void	pop(char *reg)
{
	stack_count -= 8;
	printf("    pop %s # %d -> %d\n", reg, stack_count + 8, stack_count);
}

static void	mov(char *dst, char *from)
{
	printf("    %s %s, %s\n", ASM_MOV, dst, from);
}

static void	movi(char *dst, int i)
{
	printf("    %s %s, %d\n", ASM_MOV, dst, i);
}

static void	cmps(char *dst, char *from)
{
	printf("    cmp %s, %s\n", dst, from);
}

// TODO 構造体の比較
// rax, rdi
static void	cmp(t_type *dst)
{
	if (dst->ty == TY_PTR || dst->ty == TY_ARRAY)
		cmps(RAX, RDI);
	else if (dst->ty == TY_CHAR || dst->ty == TY_BOOL)
		cmps(AL, DIL);
	else if (dst->ty == TY_INT || dst->ty == TY_ENUM)
		cmps(EAX, EDI);
	return ;
}

// RAXからR10(アドレス)に値をストアする
static void	store_value(int size)
{
	if (size == 1)
		mov("byte ptr [r10]", AL);
	else if (size == 2)
		mov("word ptr [r10]", EAX);
	else if (size == 4)
		mov("dword ptr [r10]", EAX);
	else if (size == 8)
		mov("[r10]", RAX);
	else
		error("store_valueに不正なサイズ(%d)の値を渡しています", size);
}

// アドレス(RAX)の先の値をR10(アドレス)にストアする
static void store_ptr(int size, bool minus_step)
{
	int		delta;
	char	*dest;
	char	*from;
	char	*op;
	int		i;

	op = "+";
	if (minus_step)
		op = "-";

	dest = R10;
	
	for (i = 0; i < size; i += 8)
	{
		delta = size - i;
		if (delta >= 8)
		{
			from = R11;
			printf("    %s %s, [%s + %d]\n", ASM_MOV, from, RAX, i);
			printf("    %s [%s %s %d], %s\n", ASM_MOV, dest, op, i, from);
			continue ;
		}
		if (delta >= 4)
		{
			from = R11D;
			printf("    %s %s, %s [%s + %d]\n", ASM_MOV, from, DWORD_PTR, RAX, i);
			printf("    %s %s [%s %s %d], %s\n", ASM_MOV, DWORD_PTR, dest, op,  i, from);
			i += 4;
			delta -= 4;
		}
		if (delta >= 2)
		{
			from = R11W;
			printf("    %s %s, %s [%s + %d]\n", ASM_MOV, from, WORD_PTR, RAX, i);
			printf("    %s %s [%s %s %d], %s\n", ASM_MOV, WORD_PTR, dest, op, i, from);
			i += 2;
			delta -= 2;
		}
		if (delta >= 1)
		{
			from = R11B;
			printf("    %s %s, %s [%s + %d]\n", ASM_MOV, from, BYTE_PTR, RAX, i);
			printf("    %s %s [%s %s %d], %s\n", ASM_MOV, BYTE_PTR, dest, op, i, from);
			i += 1;
			delta -= 1;
		}
	}
}

// raxをraxに読み込む
static void	load(t_type *type)
{
	if (type->ty == TY_PTR)
	{
		mov(RAX, "[rax]");
		return ;
	}
	else if (type->ty == TY_CHAR || type->ty == TY_BOOL)
	{
		printf("    mov al, %s [%s]\n", BYTE_PTR, RAX);
		printf("    movzx %s, %s\n", RAX, AL);
		return ;
	}
	else if (type->ty == TY_INT || type->ty == TY_ENUM)
	{
		printf("    mov %s, %s [%s]\n", EAX, DWORD_PTR, RAX);
		//printf("    movzx %s, %s\n", RAX, EAX);
		return ;
	}
	else if (type->ty == TY_ARRAY)
		return ;
	else if (type->ty == TY_STRUCT || type->ty == TY_UNION)
		return ;
}

// raxに入っている型fromをtoに変換する
static void	cast(t_type *from, t_type *to)
{
	int		size1;
	int		size2;
	char	*name1;
	char	*name2;

	// 型が同じなら何もしない
	if (type_equal(from, to))
		return ;

	name1 = get_type_name(from);
	name2 = get_type_name(to);
	debug("cast %s -> %s", name1, name2);

	// ポインタからポインタのキャストは何もしない
	if (is_pointer_type(from)
	&& is_pointer_type(to))
		return ;
	
	size1 = get_type_size(from);
	size2 = get_type_size(to);

	// ポインタから整数へのキャストは情報を落とす
	if (is_pointer_type(from)
	&& is_integer_type(to))
	{
		pushi(0);
		mov(R10, RSP);
		store_value(size2);
		pop(RAX);
		return ;
	}

	// 整数からポインタは0埋めで拡張
	// (ポインタはunsinged long long intなので)
	if (is_integer_type(from)
	&& is_pointer_type(to))
	{
		pushi(0);
		mov(R10, RSP);
		store_value(size1);
		pop(RAX);
		return ;
	}

	// 整数から整数は符号を考えながらキャスト
	if (is_integer_type(from)
	&& is_integer_type(to))
	{
		// TODO unsigned
		// 符号拡張する
		if (size1 < size2)
		{
			debug("cast %d -> %d", size1, size2);
			if (size1 == 1)
				printf("    movsx %s, %s\n", RAX, AL);
			else if (size1 == 4)
				printf("    movsx %s, %s\n", RAX, EAX);
			else
				error("8byte -> 8byteのキャストは無い");
		}
		return ;
	}

	if (from != NULL)
		fprintf(stderr, "from : %d\n", from->ty);
	if (to != NULL)
		fprintf(stderr, "to : %d\n", to->ty);
		
	error("%sから%sへのキャストが定義されていません\n (addr %p, %p)", name1, name2, from, to);
}

static void	print_global_constant(t_node *node, t_type *type)
{
	t_node	*notmp;

	if (type_equal(type, new_primitive_type(TY_INT)))
	{
		printf("    .long %d\n", node->val);
	}
	else if (type_equal(type, new_primitive_type(TY_CHAR)))
	{
		printf("    .byte %d\n", node->val);
	}
 	else if (type_equal(type, new_type_ptr_to(new_primitive_type(TY_CHAR)))
			&& node->def_str != NULL)
	{
		// TODO arrayのchar
		printf("    .quad L_STR_%d\n", node->def_str->index);
	}
	else if (is_pointer_type(type))
	{
		// TODO 数のチェックはしてない
		// array array
		for (notmp = node; notmp; notmp = notmp->global_assign_next)
			print_global_constant(notmp, type->ptr_to);
	}
	else
		error("print_global_constant : 未対応の型 %s", get_type_name(type));
}

static void gen_defglobal(t_defvar *node)
{
	char	*name;

	if (node->is_extern)
		return ;
	name = my_strndup(node->name, node->name_len);
	if (!node->is_static)
		printf(".globl _%s\n", name);
	if (node->assign == NULL)
	{
		printf("    .comm _%s,%d,2\n",
				name, get_type_size(node->type));
	}
	else
	{
		printf("_%s:\n", name);
		print_global_constant(node->assign, node->type);
	}
}

static void	gen_call_start(t_il *code)
{
	t_lvar	*lvar;

	debug("start call %s", my_strndup(code->funccall_callee->name, code->funccall_callee->name_len));

	g_call_locals_count = 0;

	if (is_memory_type(code->funccall_callee->type_return))
	{
		lvar				= calloc(1, sizeof(t_lvar));
		lvar->name			= "";
		lvar->name_len		= 0;
		lvar->type			= new_primitive_type(TY_INT);
		lvar->is_argument	= true;
		lvar->is_dummy		= true;
		append_argument_call(lvar);
	}
}

static void	gen_call_add_arg(t_il *code)
{
	t_lvar	*lvar;

	lvar				= calloc(1, sizeof(t_lvar));
	lvar->name			= get_type_name(code->type);
	lvar->name_len		= strlen(lvar->name);
	lvar->type			= code->type;
	lvar->is_argument	= true;
	append_argument_call(lvar);
}

static void	gen_call_exec(t_il *code)
{
	int			i;
	int			j;
	int			pop_count;
	int 		rbp_offset;
	int			size;
	t_deffunc	*deffunc;
	t_lvar		*defarg;

	int			tmp_regindex;
	int			tmp_offset;

	deffunc = code->funccall_callee;

	// 16 byteアラインチェック
	rbp_offset = 0;
	for (i = 0; i < g_call_locals_count; i++)
	{
		defarg		= g_call_locals[i];
		tmp_regindex= g_call_locals_regindex[i];
		tmp_offset	= g_call_locals_offset[i];

		if (!defarg->is_argument || tmp_regindex != -1)
			continue ;

		rbp_offset = min(rbp_offset, tmp_offset - align_to(get_type_size(defarg->type), 8) + 8);
	}

	// マイナスなのでプラスにする
	rbp_offset = - rbp_offset;
	if ((stack_count + rbp_offset) % 16 == 0)
		rbp_offset += 8;

	debug("rbp_offset : %d", rbp_offset);

	pop_count = 0;
	// 後ろから格納していく
	for (i = 0; i < g_call_locals_count; i++)
	{
		defarg			= g_call_locals[i];
		defarg->type	= type_array_to_ptr(defarg->type);
		size			= get_type_size(defarg->type);
		tmp_regindex	= g_call_locals_regindex[i];
		tmp_offset		= g_call_locals_offset[i];

		// スタックに積む
		size = align_to(size, 8);

		if (defarg->is_dummy)
			continue ;

		// レジスタに入れる
		if (tmp_regindex != -1)
		{
			if (size <= 8)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV, RAX, RSP, pop_count * 8);
				mov(arg_regs[tmp_regindex], RAX);
				pop_count += 1;
				continue ;
			}

			printf("    %s %s, [%s + %d]\n", ASM_MOV, R10, RSP, pop_count * 8);
			pop_count += 1;

			for (j = size - 8; j >= 0; j -= 8)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV,
						arg_regs[tmp_regindex - j / 8],
						R10, j);
			}
			continue ;
		}

		debug("offset : %d (%s)", tmp_offset, my_strndup(defarg->name, defarg->name_len));

		if (defarg->type->ty == TY_STRUCT || defarg->type->ty == TY_UNION)
		{
			printf("    %s %s, [%s + %d]\n", ASM_MOV, RAX, RSP, pop_count * 8);
			mov(R10, RSP);
			printf("    sub %s, %d\n", R10, (tmp_offset + 16) + rbp_offset);
			store_ptr(size, false);
		}
		else
		{
			printf("    %s %s, [%s + %d]\n", ASM_MOV, RAX, RSP, pop_count * 8);
			mov(R10, RSP);
			printf("    sub %s, %d\n", R10, (tmp_offset + 16) + rbp_offset);
			store_value(size);
		}
		pop_count += 1; // doubleなら2とかになる？
	}

	debug("ready");

	// rsp += rbp_offsetする
	printf("    sub %s, %d\n", RSP, rbp_offset);

	// 返り値がMEMORYなら、返り値の格納先のアドレスをRDIに設定する
	if (is_memory_type(deffunc->type_return))
	{
		printf("    lea %s, [%s - %d]\n", RDI, RBP, get_call_memory(code));
	}

	// call
	if (deffunc->is_variable_argument)
	{
		movi(AL, 0);
	}
	printf("    call _%s\n", my_strndup(deffunc->name, deffunc->name_len));

	// rspを元に戻す
	printf("    add %s, %d\n", RSP, rbp_offset);

	// stack_countをあわせる
	printf("    add %s, %d\n", RSP, pop_count * 8);
	stack_count -= pop_count * 8;

	// 返り値がMEMORYなら、raxにアドレスを入れる
	if (is_memory_type(deffunc->type_return))
	{
		printf("    lea %s, [%s - %d]\n", RAX, RBP, get_call_memory(code));
		push();
	}
	// 返り値がstructなら、rax, rdxを移動する
	else if (deffunc->type_return->ty == TY_STRUCT)
	{
		size = align_to(get_type_size(deffunc->type_return), 8);
		printf("    lea %s, [%s - %d]\n", RDI, RBP, get_call_memory(code));
		if (size > 0)
			printf("    mov [%s], %s\n", RDI, RAX);
		if (size > 8)
			printf("    mov [%s + 8], %s\n", RDI, RDX);
		mov(RAX, RDI);
		push();
	}
	// それ以外(int, char, ptrとか)ならraxをpushする
	else
	{
		push();
	}
}

static void	gen_macro_va_start()
{
	int		min_offset;
	int		max_argregindex;
	int		i;

	pop(RAX);

	printf("    sub rsp, 48\n");
	printf("    mov [rsp + 0], rdi\n");
	printf("    mov [rsp + 8], rsi\n");
	printf("    mov [rsp + 16], rdx\n");
	printf("    mov [rsp + 24], rcx\n");
	printf("    mov [rsp + 32], r8\n");
	printf("    mov [rsp + 40], r9\n");

	// overflow arg area
	min_offset = 0;
	max_argregindex = -1;

	for (i = 0; i < g_locals_count; i++)
	{
		min_offset = min(g_locals_offset[i], min_offset);
		max_argregindex = max(max_argregindex, g_locals_regindex[i]);
	}

	mov(RDI, RAX);

	printf("    mov dword ptr [rdi], %d\n", (1 + max_argregindex) * 8); // gp offset
	printf("    mov dword ptr [rdi + 4], 0\n\n"); // fp_offset

	printf("    mov rax, rbp\n");
	printf("    add rax, %d\n", - min_offset);
	printf("    mov [rdi + 8], rax\n"); // overflow arg area
	printf("    mov [rdi + 16], rsp\n\n"); // reg save area

	debug("    gp_offset : %d", (1 + max_argregindex) * 8);
	debug("    fp_offset : %d", 0 * 8);
}

static void	gen_save_rdi(t_il *code)
{
	t_lvar	*lvar;

	// 返り値がMEMORYなら、rdiからアドレスを取り出す
	if (is_memory_type(code->deffunc_def->type_return))
	{
		// for save rdi
		lvar				= calloc(1, sizeof(t_lvar));
		lvar->name			= "";
		lvar->name_len		= 0;
		lvar->type			= new_type_ptr_to(new_primitive_type(TY_VOID));
		lvar->is_argument	= true;
		lvar->is_dummy		= true;
		append_local(lvar);

		lvar			= calloc(1, sizeof(t_lvar));
		lvar->name		= "";
		lvar->name_len	= 0;
		lvar->type		= code->deffunc_def->type_return;
		lvar->is_dummy	= true;
		append_local(lvar);
	}
}

// 関数呼び出しを先読みして、返り値の型がMEMORYなら保存用の場所を確保する
static void gen_call_memory(t_il *code)
{
	t_type	*type_return;
	t_lvar	*lvar;

	for (; code != NULL && code->kind != IL_FUNC_EPILOGUE; code = code->next)
	{
		if (code->kind != IL_CALL_EXEC)
			continue ;
		type_return = code->funccall_callee->type_return;
		// 返り値がMEMORYなら、それを保存する用の場所を確保する
		if (is_memory_type(type_return) || type_return->ty == TY_STRUCT)
		{
			lvar				= calloc(1, sizeof(t_lvar));
			lvar->name			= "";
			lvar->name_len		= 0;
			lvar->type			= type_return;
			lvar->is_argument	= false;
			lvar->is_dummy		= true;
			append_local(lvar);
			set_call_memory(code, lvar);
		}
	}
}

static void gen_func_prologue(t_il *code)
{
	g_locals_count			= 0;
	g_call_memory_count 	= 0;

	stack_count				= 0;
	locals_stack_add		= 0;
	aligned_stack_def_var_end	= 0;

	mov(RAX, RBP);
	push();
	mov(RBP, RSP);

	gen_save_rdi(code);
	gen_call_memory(code);
}

static void	gen_func_epilogue(t_il *code)
{
	int	size;

	// MEMORYなら、rdiのアドレスに格納
	if (is_memory_type(code->type))
	{
		pop(RAX);
		printf("    mov %s, [rbp - %d]\n", R10, g_locals_offset[0]);
		store_ptr(get_type_size(code->type), false);

		// RDIを復元する
		printf("    mov %s, [rbp - %d]\n", RDI, g_locals_offset[0]);
	}
	// STRUCTなら、rax, rdxに格納
	else if (code->type->ty == TY_STRUCT)
	{
		pop(RAX);
		size = align_to(get_type_size(code->type), 8);
		if (size > 8)
			printf("    mov %s, [%s  - 8]\n", RDX, RAX);
		if (size > 0)
			printf("    mov %s, [%s]\n", RAX, RAX);
	}
	else if (code->type->ty != TY_VOID)
		pop(RAX);

	printf("    add %s, %d # %d\n", RSP, aligned_stack_def_var_end, stack_count);
	stack_count -= aligned_stack_def_var_end;
	printf("    add %s, %d # %d\n", RSP, locals_stack_add, stack_count);
	stack_count -= locals_stack_add;

	mov(RSP, RBP);
	pop(RBP);
	printf("    ret\n");

	g_locals_count = 0;

	if (stack_count != 0)
	{
		fprintf(stderr, "STACKCOUNT err %d\n", stack_count);
		error("Error");
	}

	stack_count = 0;
}

static void	gen_def_var_local(t_il *code)
{
	append_local(code->var_local);
}

static void	gen_def_var_end(void)
{
	int	old;

	old			= stack_count;
	stack_count	= align_to(stack_count, 16);

	printf("    sub %s, %d\n", RSP, stack_count - old);
	aligned_stack_def_var_end = stack_count - old;
}

static void	gen_var_local_addr(t_il *code)
{
	int	i;
	int	offset;

	offset = 0;
	for (i = 0; i < g_locals_count; i++)
	{
		if (g_locals[i] == code->var_local)
		{
			offset = g_locals_offset[i];
			break ;
		}
	}

	mov(RAX, RBP);
	if (offset >= 0)
		printf("    sub %s, %d\n", RAX, offset);
	else
		printf("    add %s, %d\n", RAX, -offset);
	push();
}

static void	gen_il(t_il *code)
{
	//printf("# kind %d\n", code->kind);
	switch (code->kind)
	{
		case IL_LABEL:
		{
			if (code->label_is_deffunc)
			{
				if (!code->label_is_static_func)
					printf(".globl _%s\n", code->label_str);
				printf("_%s:\n", code->label_str);
			}
			else	
				printf("%s:\n", code->label_str);
			return ;
		}
		case IL_JUMP:
		{
			printf("    jmp %s\n", code->label_str);
			return ;
		}
		case IL_JUMP_EQUAL:
		{
			pop(RAX);
			printf("    je %s\n", code->label_str);
			return ;
		}
		case IL_JUMP_NEQUAL:
		{
			pop(RAX);
			printf("    jne %s\n", code->label_str);
			return;
		}
		case IL_FUNC_PROLOGUE:
			gen_func_prologue(code);
			return ;
		case IL_FUNC_EPILOGUE:
			gen_func_epilogue(code);
			return ;
		case IL_DEF_VAR_LOCAL:
			gen_def_var_local(code);
			return ;
		case IL_DEF_VAR_END:
			gen_def_var_end();
			return ;

		case IL_PUSH_AGAIN:
		{
			// TODO 型
			debug("AGAIN");
			push();
			return ;
		}
		case IL_PUSH_NUM:
			pushi(code->number_int);
			return ;
		case IL_STACK_SWAP:
		{
			// とりあえず型を何も考えずに交換する
			pop(RAX);
			mov(R10, RAX);

			pop(RDI);
			mov(RAX, R10);
			push();

			mov(RAX, RDI);
			push();
			return ;
		}
		case IL_POP:
			// TODO 型
			pop(RAX);
			return ;

		// TODO 型を考慮
		case IL_ADD:
			pop(RDI);
			pop(RAX);
			printf("    add %s, %s\n", RAX, RDI);
			push();
			return ;
		case IL_SUB:
			pop(RDI);
			pop(RAX);
			printf("    sub %s, %s\n", RAX, RDI);
			push();
			return ;
		case IL_MUL:
			pop(RDI);
			pop(RAX);
			printf("    movsxd %s, %s\n", RAX, EAX);
			printf("    movsxd %s, %s\n", RDI, EDI);
			printf("    imul %s, %s\n", RAX, RDI);	
			push();
			return ;
		case IL_DIV:
			pop(RDI);
			pop(RAX);
			printf("    cdq\n");
			printf("    idiv edi\n");
			push();
			return ;
		case IL_MOD:
			pop(RDI);
			pop(RAX);
			printf("    cdq\n"); // d -> q -> o
			printf("    idiv edi\n");
			mov(RAX, RDX);
			push();
			return ;

		case IL_EQUAL:
			pop(RDI);
			pop(RAX);
			cmp(code->type);
			printf("    sete al\n");
			printf("    movzx rax, al\n");
			push();
			return ;
		case IL_NEQUAL:
			pop(RDI);
			pop(RAX);
			cmp(code->type);
			printf("    setne al\n");
			printf("    movzx rax, al\n");
			push();
			return ;
		case IL_LESS:
			pop(RDI);
			pop(RAX);
			cmp(code->type);
			printf("    setl al\n");
			printf("    movzx rax, al\n");
			push();
			return ;
		case IL_LESSEQ:
			pop(RDI);
			pop(RAX);
			cmp(code->type);
			printf("    setle al\n");
			printf("    movzx rax, al\n");
			push();
			return ;
		case IL_BITWISE_AND:
			pop(RAX);
			pop(RDI);
			printf("   and %s, %s\n", RAX, RDI);
			push();
			return ;
		case IL_BITWISE_XOR:
			pop(RAX);
			pop(RDI);
			printf("   xor %s, %s\n", RAX, RDI);
			push();
			return ;
		case IL_BITWISE_OR:
			pop(RAX);
			pop(RDI);
			printf("    or %s, %s\n", RAX, RDI);
			push();
			return ;
		case IL_BITWISE_NOT:
			pop(RAX);
			printf("    xor %s, -1\n", RAX);
			push();
			return;
		case IL_SHIFT_RIGHT:
			pop(RDI);
			pop(RAX);
			mov(CL, DIL);
			printf("    shr %s, %s\n", RAX, CL);
			push();
			return;
		case IL_SHIFT_LEFT:
			pop(RDI);
			pop(RAX);
			mov(CL, DIL);
			printf("    shl %s, %s\n", RAX, CL);
			push();
			return;

		case IL_ASSIGN:
			pop(RAX);
			pop(R10);
			// TODO ARRAYに対する代入 ? 
			if (code->type->ty == TY_ARRAY)
			{
				store_value(8);
				push();
			}
			else if(code->type->ty == TY_STRUCT || code->type->ty == TY_UNION)
			{
				store_ptr(get_type_size(code->type), false);
				push(); // TODO これOK?
			}
			else
			{
				store_value(get_type_size(code->type));
				push();
			}
			return ;

		case IL_VAR_LOCAL:
			code->kind = IL_VAR_LOCAL_ADDR;
			gen_il(code);
			code->kind = IL_VAR_LOCAL;
			pop(RAX);
			load(code->var_local->type);
			push();
			return ;
		case IL_VAR_LOCAL_ADDR:
			gen_var_local_addr(code);
			return ;
		case IL_VAR_GLOBAL:
			code->kind = IL_VAR_GLOBAL_ADDR;
			gen_il(code);
			code->kind = IL_VAR_GLOBAL;
			pop(RAX);
			load(code->var_global->type);
			push();
			return ;
		case IL_VAR_GLOBAL_ADDR:
			printf("    mov rax, [rip + _%s@GOTPCREL]\n",
					my_strndup(code->var_global->name, code->var_global->name_len));
			push();
			return ;
		// TODO genでoffsetの解決 , analyzeで宣言のチェック
		case IL_MEMBER:
		case IL_MEMBER_PTR:
			pop(RAX);
			printf("    add %s, %d\n", RAX, get_member_offset(code->member));
			load(code->member->type);
			push();
			return ;
		case IL_MEMBER_ADDR:
		case IL_MEMBER_PTR_ADDR:
			pop(RAX);
			printf("    add %s, %d\n", RAX, get_member_offset(code->member));
			push();
			return ;
		case IL_STR_LIT:
			printf("    %s %s, [rip + L_STR_%d]\n", ASM_LEA, RAX, code->def_str->index);
			push();
			return;

		case IL_CALL_START:
			gen_call_start(code);
			return ;
		case IL_CALL_ADD_ARG:
			gen_call_add_arg(code);
			return ;
		case IL_CALL_EXEC:
			gen_call_exec(code);
			return ;
		case IL_MACRO_VASTART:
			gen_macro_va_start();
			return ;

		case IL_CAST:
			// TODO とりあえずpop / pushしてます
			printf("#CAST\n");
			pop(RAX);
			cast(code->cast_from, code->cast_to);
			push();
			printf("#CAST END\n");
			return ;

		case IL_LOAD:
			// TODO とりあえずpop / pushしてます
			pop(RAX);
			load(code->type);
			push();
			return ;

		default:
		{
			fprintf(stderr, "gen not implemented\nkind : %d\n", code->kind);
			error("Error");
			return ;
		}
	}
}

void	codegen_riscv(void)
{
	int		i;
	t_il	*code;

	printf(".intel_syntax noprefix\n");
	printf(".p2align	4, 0x90\n");

	// 文字列リテラル生成
	for (i = 0; g_str_literals[i] != NULL; i++)
	{
		printf("L_STR_%d:\n", g_str_literals[i]->index);
		printf("    .string \"");
		put_str_literal(g_str_literals[i]->str, g_str_literals[i]->len);
		printf("\"\n");
	}

	// グローバル変数を生成
	printf(".section	__DATA, __data\n");
	for (i = 0; g_global_vars[i] != NULL; i++)
		gen_defglobal(g_global_vars[i]);

	// コードを生成
	printf(".section	__TEXT,__text,regular,pure_instructions\n");
	for (code = g_il; code != NULL; code = code->next)
		gen_il(code);
}
