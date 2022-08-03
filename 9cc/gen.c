#include "9cc.h"
#include "il.h"
#include "charutil.h"
#include "mymath.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

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

static char				*arg_regs[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static int				stack_count = 0;

// main
extern t_deffunc		*g_func_defs[1000];
extern t_defvar			*g_global_vars[1000];
extern t_str_elem		*g_str_literals[1000];
extern t_il				*g_il;

static void	push()
{
	stack_count += 8;
	
	debug("stack= %d -> %d", stack_count - 8, stack_count);
	printf("    %s %s\n", ASM_PUSH, RAX);
}

static void	pushi(int num)
{
	stack_count += 8;
	debug("stack= %d -> %d", stack_count - 8, stack_count);
	printf("    %s %d\n", ASM_PUSH, num);
}

static void	pop(char *reg)
{
	stack_count -= 8;
	debug("stack= %d -> %d", stack_count + 8, stack_count);
	printf("    pop %s\n", reg);
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
			if (i != 0)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV, from, RAX, i);
				printf("    %s [%s %s %d], %s\n", ASM_MOV, dest, op, i, from);
			}
			else
			{
				printf("    %s %s, [%s]\n", ASM_MOV, from, RAX);
				printf("    %s [%s], %s\n", ASM_MOV, dest, from);
			}
			continue ;
		}
		if (delta >= 4)
		{
			from = R11D;
			if (i != 0)
			{
				printf("    %s %s, %s [%s + %d]\n", ASM_MOV, from, DWORD_PTR, RAX, i);
				printf("    %s %s [%s %s %d], %s\n", ASM_MOV, DWORD_PTR, dest, op,  i, from);
			}
			else
			{
				printf("    %s %s, %s [%s]\n", ASM_MOV, from, DWORD_PTR, RAX);
				printf("    %s %s [%s], %s\n", ASM_MOV, DWORD_PTR, dest, from);
			}
			i += 4;
			delta -= 4;
		}
		if (delta >= 2)
		{
			from = R11W;
			if (i != 0)
			{
				printf("    %s %s, %s [%s + %d]\n", ASM_MOV, from, WORD_PTR, RAX, i);
				printf("    %s %s [%s %s %d], %s\n", ASM_MOV, WORD_PTR, dest, op, i, from);
			}
			else
			{
				printf("    %s %s, %s [%s]\n", ASM_MOV, from, WORD_PTR, RAX);
				printf("    %s %s [%s], %s\n", ASM_MOV, WORD_PTR, dest, from);
			}
			i += 2;
			delta -= 2;
		}
		if (delta >= 1)
		{
			from = R11B;
			if (i != 0)
			{
				printf("    %s %s, %s [%s + %d]\n", ASM_MOV, from, BYTE_PTR, RAX, i);
				printf("    %s %s [%s %s %d], %s\n", ASM_MOV, BYTE_PTR, dest, op, i, from);
			}
			else
			{
				printf("    %s %s, %s [%s]\n", ASM_MOV, from, BYTE_PTR, RAX);
				printf("    %s %s [%s], %s\n", ASM_MOV, BYTE_PTR, dest, from);
			}
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

/* rax, rdi
static void	create_add(bool is_add, t_type *l, t_type *r)
{
	int	size;

	debug("    add(%d) %s(%d) + %s(%d)", is_add,
			get_type_name(l), get_type_size(l),
			get_type_name(r), get_type_size(r));

	if (is_integer_type(l))
	{
		size = get_type_size(l);
		if (size == 4)
			printf("    movsxd rax, eax\n");
		else if (size == 2)
			printf("    movsx rax, ax\n");
		else if (size == 1)
			printf("    movsx rax, al\n");
	}

	if (is_integer_type(r))
	{
		size = get_type_size(r);
		if (size == 4)
			printf("    movsxd rdi, edi\n");
		else if (size == 2)
			printf("    movsx rdi, di\n");
		else if (size == 1)
			printf("    movsx rdi, dil\n");
	}

	if (is_add)
		printf("    add rax, rdi\n");
	else
		printf("    sub rax, rdi\n");
}*/

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
	name = strndup(node->name, node->name_len);
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

static t_lvar	*g_funcnow_locals;
static int		g_funcnow_offset;

static void	gen_call_start(t_il *code)
{
	debug("start %s", strndup(code->funccall_callee->name, code->funccall_callee->name_len));
}

static void	gen_call_add_arg(t_il *code)
{
	int		i;
	int		size;
	t_lvar	*def;

	size	= get_type_size(code->type);
	def		= code->funccall_arg_def;

	debug("PUSH ARG %s(%d)", strndup(def->name, def->name_len), size);

	// popする TODO floatとかは?
	pop(RAX);

	if (def->arg_regindex != -1)
	{
		if (size <= 8)
		{
			push();
			return ;
		}
		// size > 8はstructなので、スタックに積む
		mov(RDI, RAX);
		for (i = 0; i < size; i += 8)
		{
			if (i == 0)
				printf("    %s %s, [%s]\n", ASM_MOV, RAX, RDI);
			else
				printf("    %s %s, [%s + %d]\n", ASM_MOV, RAX, RDI, i);
			push();
		}
	}
	else
		push();
}

static void	gen_call_exec(t_il *code)
{
	int			i;
	int			j;
	int			pop_count;
	int 		rbp_offset;
	bool		is_aligned;
	int			size;
	t_deffunc	*deffunc;
	t_lvar		*defarg;

	deffunc = code->funccall_callee;

	// 16 byteアラインチェック
	rbp_offset = 0;
	for (i = 0; i < code->funccall_argcount; i++)
	{
		defarg = code->funccall_argdefs[i];
		if (defarg->arg_regindex != -1)
			continue ;
		rbp_offset = min(rbp_offset, defarg->offset - align_to(get_type_size(defarg->type), 8) + 8);
	}

	// マイナスなのでプラスにする
	rbp_offset = - rbp_offset;

	is_aligned = (stack_count + rbp_offset + 8) % 16 == 0;
	if (!is_aligned)
		rbp_offset += 8;

	debug("RBP_OFFSET %d (is_aligned : %d)", rbp_offset, is_aligned);
	
	pop_count = 0;
	// 後ろから格納していく
	for (i = code->funccall_argcount - 1; i >= 0; i--)
	{
		defarg			= code->funccall_argdefs[i];
		defarg->type	= type_array_to_ptr(defarg->type);
		size			= get_type_size(defarg->type);

		debug("POP %s", strndup(defarg->name, defarg->name_len));

		// レジスタに入れる
		if (defarg->arg_regindex != -1)
		{
			if (size <= 8)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV, RAX, RSP, pop_count++ * 8);
				mov(arg_regs[defarg->arg_regindex], RAX);
				continue ;
			}
			for (j = size - 8; j >= 0; j -= 8)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV,
					arg_regs[defarg->arg_regindex - j / 8],
					 RSP, pop_count++ * 8);
			}
			continue ;
		}

		debug("OFFSET %d", defarg->offset);

		// スタックに積む
		// 必ず8byteアラインなので楽々実装
		size = align_to(size, 8);

		printf("    %s %s, [%s + %d]\n", ASM_MOV, RAX, RSP, pop_count++ * 8);
		mov(R10, RSP);

		printf("    sub %s, %d\n", R10,
				(defarg->offset + 16) + rbp_offset);

		// STRUCTならptr先を渡す
		if (defarg->type->ty == TY_STRUCT || defarg->type->ty == TY_UNION)
			store_ptr(size, false);
		else
			store_value(size);
	}

	// rspを移動する
	if (rbp_offset != 0)
	{
		if (!is_aligned)
			debug("aligned + 8");
		debug("rbp_offset");
		printf("    sub %s, %d\n", RSP, rbp_offset);
	}

	// 返り値がMEMORYなら、返り値の格納先のアドレスをRDIに設定する
	if (is_memory_type(deffunc->type_return))
	{
		printf("    lea %s, [%s - %d]\n", RDI, RBP, code->funccall_save_pos->offset);
	}

	// call
	debug("CALL RBP_OFFSET: %d", rbp_offset);
	if (deffunc->is_variable_argument)
		movi(AL, 0);
	printf("    call _%s\n", strndup(deffunc->name, deffunc->name_len));

	// rspを元に戻す
	if (rbp_offset != 0)
	{
		debug("rbp_offset");
		printf("    add %s, %d\n", RSP, rbp_offset);
	}

	// stack_countをあわせる
	debug("pop_count");
	printf("    add %s, %d\n", RSP, pop_count * 8);
	debug("POP ALL %d -> %d", stack_count, stack_count - pop_count * 8);
	stack_count -= pop_count * 8;

	// 返り値がMEMORYなら、raxにアドレスを入れる
	if (is_memory_type(deffunc->type_return))
	{
		printf("    lea %s, [%s - %d]\n", RAX, RBP, code->funccall_save_pos->offset);
		push();
	}
	// 返り値がstructなら、rax, rdxを移動する
	else if (deffunc->type_return->ty == TY_STRUCT)
	{
		size = align_to(get_type_size(deffunc->type_return), 8);
		printf("    lea %s, [%s - %d]\n", RDI, RBP, code->funccall_save_pos->offset);
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

static void	gen_macro_va_start(t_il *code)
{
	int		min_offset;
	int		max_argregindex;
	t_lvar	*def;

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

	for (def = code->funccall_caller->locals; def; def = def->next)
	{
		min_offset = min(def->offset, min_offset);
		max_argregindex = max(max_argregindex, def->arg_regindex);
		debug(" # LOOP %d\n", def->arg_regindex);
	}

	mov(RDI, RAX);

	printf("    mov dword ptr [rdi], %d\n", (1 + max_argregindex) * 8); // gp offset
	printf("    mov dword ptr [rdi + 4], 0\n\n"); // fp_offset

	printf("    mov rax, rbp\n");
	printf("    add rax, %d\n", - min_offset);
	printf("    mov [rdi + 8], rax\n"); // overflow arg area
	printf("    mov [rdi + 16], rsp\n\n"); // reg save area

	debug("    VA_START");
	debug("    gp_offset : %d", (1 + max_argregindex) * 8);
	debug("    fp_offset : %d", 0 * 8);
}

static void gen_func_prologue(t_il *code)
{
	stack_count = 0;

	mov(RAX, RBP);
	push();
	mov(RBP, RSP);
	g_funcnow_locals = NULL;
	g_funcnow_offset = 0;

	// 返り値がMEMORYなら、rdiからアドレスを取り出す
	if (is_memory_type(code->deffunc_def->type_return))
	{
		printf("    mov [rbp - %d], %s\n", code->deffunc_def->locals->offset, RDI);
	}	
}

static void	gen_func_epilogue(t_il *code)
{
	int	size;

	// MEMORYなら、rdiのアドレスに格納
	if (is_memory_type(code->type))
	{
		printf("    mov %s, [rbp - %d]\n", R10, code->deffunc_def->locals->offset);
		store_ptr(get_type_size(code->type), false);

		// RDIを復元する
		printf("    mov %s, [rbp - %d]\n", RDI, code->deffunc_def->locals->offset);
	}
	// STRUCTなら、rax, rdxに格納
	else if (code->type->ty == TY_STRUCT)
	{
		size = align_to(get_type_size(code->type), 8);
		if (size > 8)
			printf("    mov %s, [%s  - 8]\n", RDX, RAX);
		if (size > 0)
			printf("    mov %s, [%s]\n", RAX, RAX);
	}
	else if (code->type->ty != TY_VOID)
		pop(RAX);

	mov(RSP, RBP);
	pop(RBP);
	printf("    ret\n");

	g_funcnow_locals = NULL;

	stack_count = 0;
}

static void	gen_def_var_local(t_il *code)
{
	t_lvar	*def;
	int		size;
	int		index;
	int		offset_before;

	// TODO 引数の配列型
	def		= code->var_local;
	size	= get_type_size(def->type);

	// offsetを設定する
	if (!def->is_arg || def->arg_regindex != -1)
	{
		offset_before	= g_funcnow_offset;
		g_funcnow_offset= align_to(g_funcnow_offset + size, 8);
		def->offset		= g_funcnow_offset;

		printf("    sub %s, %d\n", RSP, g_funcnow_offset - offset_before);
		stack_count += g_funcnow_offset - offset_before;
	}

	if (def->is_arg)
	{
		debug("ARG %s", strndup(def->name, def->name_len));
		if (def->arg_regindex != -1)
		{
			index = def->arg_regindex;
			size = align_to(get_type_size(def->type), 8);
	
			mov(R10, RBP);
			printf("    sub %s, %d\n", R10, def->offset);
	
			// とりあえず、かならず8byte境界になっている
			// ↑ は？
			while (size >= 8)
			{
				mov(RAX, arg_regs[index--]);
				store_value(8);
				size -= 8;
				if (size != 0)
				printf("    add %s, 8\n", R10);
			}
		}
	}
}

static void	gen_def_var_end(void)
{
	int	to;

	to = align_to(stack_count, 16);
	if (stack_count != to)
		printf("    sub %s, %d\n", RSP, to - stack_count);
	stack_count = to;
}

static void	gen_il(t_il *code)
{
	printf("# kind %d\n", code->kind);
	switch (code->kind)
	{
		case IL_LABEL:
		{
			if (code->label_is_deffunc)
			{
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
			printf("    je %s\n", code->label_str);
			return ;
		}
		case IL_JUMP_NEQUAL:
		{
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
			mov(RAX, RDI);
			push();

			mov(RAX, R10);
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
			printf("    imul rax, rdi\n");	
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
				store_ptr(get_type_size(code->type), false);
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
			mov(RAX, RBP);
			if (code->var_local->offset > 0)
				printf("    sub %s, %d\n", RAX, code->var_local->offset);
			else if (code->var_local->offset < 0)
				printf("    add %s, %d\n", RAX, -code->var_local->offset);
			push();
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
					strndup(code->var_global->name, code->var_global->name_len));
			push();
			return ;
		// TODO genでoffsetの解決 , analyzeで宣言のチェック
		case IL_MEMBER:
		case IL_MEMBER_PTR:
			pop(RAX);
			printf("    add %s, %d\n", RAX, code->member->offset);
			load(code->member->type);
			push();
			return ;
		case IL_MEMBER_ADDR:
		case IL_MEMBER_PTR_ADDR:
			pop(RAX);
			printf("    add %s, %d\n", RAX, code->member->offset);
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
			gen_macro_va_start(code);
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

void	codegen(void)
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
