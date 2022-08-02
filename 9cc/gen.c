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
static void	lval(t_node *node);
static void	call(t_node *node);
static void	cast(t_type *from, t_type *to);
static void	arrow(t_node *node, bool as_addr);
static void	create_add(bool is_add, t_type *l, t_type *r);
static void	load_lval_addr(t_node *node);
static void	print_global_constant(t_node *node, t_type *type);
static void	gen(t_node *node);

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
	{
		return ;
	}
	else if (type->ty == TY_STRUCT || type->ty == TY_UNION)
	{
		// TODO とりあえず8byteまで
		// mov(RAX, "[rax]");
		return ;
	}
}

// 変数のアドレスをraxに移動する
static void	lval(t_node *node)
{
	int	offset;

	if (node->kind != ND_VAR_LOCAL && node->kind != ND_VAR_GLOBAL)
		error("変数ではありません Kind:%d t_type:%d", node->kind, node->type->ty);

	if (node->kind == ND_VAR_LOCAL)
	{
		offset = node->lvar->offset;
		mov(RAX, RBP);
		if (offset > 0)
			printf("    sub %s, %d\n", RAX, offset);
		else if (offset < 0)
			printf("    add %s, %d\n", RAX, - offset);
	}
	else if (node->kind == ND_VAR_GLOBAL)
	{
		printf("    mov rax, [rip + _%s@GOTPCREL]\n", strndup(node->var_global->name, node->var_global->name_len));
	}
}

static void	call(t_node *node)
{
	int		size;
	int 	rbp_offset;
	int		push_count = 0;
	bool	is_aligned;
	t_node	*tmp;
	int		i;
	int		j;
	int		count;
	int		pop_count;

	t_lvar	*lvtmp;
	int		min_offset;
	int		max_argregindex;

	// va_startマクロ
	if (node->funcdef->name_len == 8
	&& strncmp(node->funcdef->name, "va_start", 8) == 0)
	{
		// register save
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

		// TODO g_func_nowをcallで設定する

		for (lvtmp = g_func_now->locals; lvtmp; lvtmp = lvtmp->next)
		{
			min_offset = min(lvtmp->offset, min_offset);
			max_argregindex = max(max_argregindex, lvtmp->arg_regindex);
			debug(" # LOOP %d\n", lvtmp->arg_regindex);
		}

		// 一つめの引数を読み込み
		if (node->funccall_argcount == 0)
			error("va_startに引数がありません");

		lval(node->funccall_args[0]);
		mov(RDI, RAX);

		printf("    mov dword ptr [rdi], %d\n", (1 + max_argregindex) * 8); // gp offset
		printf("    mov dword ptr [rdi + 4], 0\n\n"); // fp_offset

		printf("    mov rax, rbp\n");
		printf("    add rax, %d\n", - min_offset);
		printf("    mov [rdi + 8], rax\n\n"); // overflow arg area

		printf("    mov [rdi + 16], rsp\n\n"); // reg save area

		debug("    VA_START");
		debug("    gp_offset : %d", (1 + max_argregindex) * 8);
		debug("    fp_offset : %d", 0 * 8);

		return ;
	}

	t_lvar	*lvar_defargs;

	for (count = 0; count < node->funccall_argcount; count++)
	{
		tmp = node->funccall_args[count];
		lvar_defargs = node->funccall_argdefs[count];

		size = get_type_size(type_array_to_ptr(lvar_defargs->type));

		debug("PUSH ARG %s (%d)",
			strndup(lvar_defargs->name, lvar_defargs->name_len),size);

		gen(tmp);

		if (lvar_defargs->arg_regindex != -1)
		{
			if (size <= 8)
			{
				push();
				push_count++;
				continue ;
			}
			// size > 8はstructなので、スタックに積む
			mov(RDI, RAX);
			for (i = 0; i < size; i += 8)
			{
				if (i == 0)
					printf("    %s %s, [%s]\n",
						ASM_MOV, RAX, RDI);
				else
					printf("    %s %s, [%s + %d]\n",
						ASM_MOV, RAX, RDI, i);
				push();
				push_count++;
			}
		}
		else
		{
			push();
			push_count++;
		}
	}

	// 16 byteアラインチェック
	rbp_offset = 0;
	for (i = 0; i < node->funccall_argcount; i++)
	{
		tmp = node->funccall_args[i];
		lvar_defargs = node->funccall_argdefs[i];

		if (lvar_defargs->arg_regindex != -1)
			continue ;
		rbp_offset = min(rbp_offset, lvar_defargs->offset - align_to(get_type_size(lvar_defargs->type), 8) + 8);
	}

	// マイナスなのでプラスにする
	rbp_offset = - rbp_offset;

	is_aligned = (stack_count + rbp_offset + 8) % 16 == 0;
	if (!is_aligned)
		rbp_offset += 8;

	debug("RBP_OFFSET %d (is_aligned : %d)", rbp_offset, is_aligned);
	
	pop_count = 0;
	// 後ろから格納していく
	for (i = node->funccall_argcount - 1; i >= 0; i--)
	{
		tmp = node->funccall_args[i];
		lvar_defargs = node->funccall_argdefs[i];

		debug("POP %s", strndup(lvar_defargs->name, lvar_defargs->name_len));
	
		size = get_type_size(type_array_to_ptr(lvar_defargs->type));

		// レジスタに入れる
		if (lvar_defargs->arg_regindex != -1)
		{
			if (size <= 8)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV, RAX, RSP, pop_count++ * 8);
				mov(arg_regs[lvar_defargs->arg_regindex], RAX);
				continue ;
			}
			// size > 8なものは必ずstructであると願います( ;∀;)
			// RAXにアドレスが入っていると想定
			for (j = size - 8; j >= 0; j -= 8)
			{
				printf("    %s %s, [%s + %d]\n", ASM_MOV,
					arg_regs[lvar_defargs->arg_regindex - j / 8],
					 RSP, pop_count++ * 8);
			}
			continue ;
		}

		debug("OFFSET %d", lvar_defargs->offset);

		// スタックに積む
		// 必ず8byteアラインなので楽々実装
		size = align_to(size, 8);

		printf("    %s %s, [%s + %d]\n", ASM_MOV, RAX, RSP, pop_count++ * 8);
		mov(R10, RSP);

		printf("    sub %s, %d\n", R10,
				(lvar_defargs->offset + 16) + rbp_offset);

		// ptr先を渡すのはSTRUCTだけ
		if (tmp->type->ty == TY_STRUCT
			|| tmp->type->ty == TY_UNION)
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
	if (is_memory_type(node->type))
	{
		printf("    lea %s, [%s - %d]\n", RDI, RBP, node->call_mem_stack->offset);
	}

	// call
	debug("CALL RBP_OFFSET: %d", rbp_offset);
	if (node->funcdef->is_variable_argument)
		movi(AL, 0);
		printf("    mov %s, 0\n", AL);
	printf("    call _%s\n", strndup(node->funcdef->name, node->funcdef->name_len));

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
	if (is_memory_type(node->type))
	{
		printf("    lea %s, [%s - %d]\n", RAX, RBP, node->call_mem_stack->offset);
	}
	// 返り値がstructなら、rax, rdxを移動する
	else if (node->type->ty == TY_STRUCT)
	{
		size = align_to(get_type_size(node->type), 8);
		printf("    lea %s, [%s - %d]\n", RDI, RBP, node->call_mem_stack->offset);
		if (size > 0)
			printf("    mov [%s], %s\n", RDI, RAX);
		if (size > 8)
			printf("    mov [%s + 8], %s\n", RDI, RDX);
		mov(RAX, RDI);
	}
	return;
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

static void	arrow(t_node *node, bool as_addr)
{
	int offset;

	// arrowかその他の可能性がある
	if (node->lhs->kind == ND_MEMBER_VALUE
	|| node->lhs->kind == ND_MEMBER_PTR_VALUE)
		arrow(node->lhs, node->kind == ND_MEMBER_VALUE);
	else
		gen(node->lhs);

	// offsetを足す
	offset = node->elem->offset;
	debug("offset");
	printf("    add rax, %d\n", offset);

	// 値として欲しいなら値にする
	if (!as_addr)
		load(node->elem->type);
}

static void	mul_prologue(t_node *node)
{
	gen(node->rhs);
	push();
	gen(node->lhs);
	pop("rdi");
}

// rax, rdi
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
}

static void	relational_prologue(t_node *node)
{
	gen(node->rhs);
	push();
	gen(node->lhs);
	pop("rdi");
}

static void	equality_prologue(t_node *node)
{
	gen(node->rhs);
	push();
	gen(node->lhs);
	pop("rdi");
}

static void	assign_prologue(t_node *node)
{
	debug("ASSIGN %d", node->lhs->kind);
	load_lval_addr(node->lhs);	
	push();
}

static void	load_lval_addr(t_node *node)
{
	if (node->kind == ND_VAR_LOCAL)
		lval(node);
	else if (node->kind == ND_VAR_GLOBAL)
		lval(node);
	else if (node->kind == ND_DEREF)
		gen(node->lhs);// ここもDEREFと同じようにやってる！！！！！
	else if (node->kind == ND_MEMBER_VALUE)
		arrow(node, true);
	else if (node->kind == ND_MEMBER_PTR_VALUE)
		arrow(node, true);
	else if (node->kind == ND_PARENTHESES)
		load_lval_addr(node->lhs);
	else
		error("左辺値が識別子かアドレスではありません");
}

static void	assign_epilogue(t_type *type)
{
	pop("r10");

	// storeする
	if (type->ty == TY_ARRAY)
		// TODO これOKなの？
		// ARRAYに対する代入がうまくいかない気がする
		store_value(8);
	else if(type->ty == TY_STRUCT
			|| type->ty == TY_UNION)
		store_ptr(get_type_size(type), false);
	else
		store_value(get_type_size(type));
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

static void	gen(t_node *node)
{
	int				lend;
	int				lbegin;
	int				lbegin2;
	int				lelse;
	t_switchcase	*cases;

	switch(node->kind)
	{
		case ND_CASE:
		{
			debug("case %d", node->val);
			printf(".Lswitch%d:\n", node->case_label->label);
			return ;
		}
		case ND_BREAK:
		{
			debug("break");
			printf("jmp .Lend%d\n", node->block_sbdata->endLabel);
			return ;
		}
		case ND_CONTINUE:
		{
			debug("continue");
			printf("jmp .Lbegin%d\n", node->block_sbdata->startLabel);
			return ;
		}
		case ND_DEFAULT:
		{
			debug("default");
			printf(".Lswitch%d:\n", node->block_sbdata->defaultLabel);
			return ;
		}
		case ND_ASSIGN:
		{
			assign_prologue(node);
			gen(node->rhs);
			assign_epilogue(node->type);
			return ;
		}
		case ND_COMP_ADD:
		{
			assign_prologue(node);

			push();

			gen(node->rhs);
			pop(RDI);

			if (node->type->ty == TY_ARRAY)
				printf("    add rax, rdi\n");
			else
				printf("    add rax, [rdi]\n");

			assign_epilogue(node->type);
			return ;
		}
		case ND_COMP_SUB:
		{
			assign_prologue(node);

			push();

			gen(node->rhs);
			mov(RDI, RAX);
			pop(RAX);
			if (node->type->ty == TY_ARRAY)
				printf("    sub rax, rdi\n");
			else
			{
				printf("    mov rax, [rax]\n");
				printf("    sub rax, rdi\n");
			}

			assign_epilogue(node->type);
			return ;
		}
		case ND_COMP_MUL:
		{
			assign_prologue(node);

			push();
			gen(node->rhs);
			mov(RDI, RAX);
			pop(RAX);
			// TODO 整数だけ？
			printf("    mov rax, [rax]\n");
			printf("    imul rax, rdi\n");

			assign_epilogue(node->type);
			return;
		}
		case ND_COMP_DIV:
		{
			assign_prologue(node);

			push();
			gen(node->rhs);
			mov(RDI, RAX);
			pop(RAX);
			// TODO 整数だけ？
			printf("    mov rax, [rax]\n");
			printf("    cdq\n");
			printf("    idiv edi\n");

			assign_epilogue(node->type);
			return ;
		}
		case ND_COMP_MOD:
		{
			assign_prologue(node);

			push();
			gen(node->rhs);
			mov(RDI, RAX);
			pop(RAX);
			// TODO 整数だけ？
			printf("    mov rax, [rax]\n");
			printf("    cdq\n");
			printf("    idiv edi\n");
			mov(RAX, RDX);

			assign_epilogue(node->type);
			return ;
		}
		case ND_COND_AND:
		{
			lend = jumpLabelCount++;

			// 左辺を評価
			gen(node->lhs);
			mov(RDI, "0");
			cmp(node->lhs->type);
			printf("    setne al\n"); // 0と等しくないかをalに格納
			printf("    je .Lcond%d\n", lend); // 0ならスキップ

			push();

			// 右辺を評価
			gen(node->rhs);
			mov(RDI, "0");
			cmp(node->rhs->type);
			printf("    setne al\n"); // 0と等しくないかをalに格納

			pop(RDI);
		
			create_add(true, new_primitive_type(TY_INT), new_primitive_type(TY_INT));
			printf("    movzx rax, al\n"); // alをゼロ拡張

			// 最後の比較
			printf(".Lcond%d:\n", lend);
			mov(RDI, "2");
			cmp(new_primitive_type(TY_INT));
			printf("    sete al\n"); // 2と等しいかをalに格納
			printf("    movzx rax, al\n"); // alをゼロ拡張
			return ;
		}
		case ND_COND_OR:
		{
			lend = jumpLabelCount++;

			// 左辺を評価
			gen(node->lhs);
			mov(RDI, "0");
			cmp(node->lhs->type);
			printf("    setne al\n"); // 0と等しくないかをalに格納
			printf("    movzx rax, al\n"); // alをゼロ拡張
			printf("    jne .Lcond%d\n", lend); // 0以外ならスキップ

			// 右辺を評価
			gen(node->rhs);
			mov(RDI, "0");
			cmp(node->rhs->type);
			printf("    setne al\n"); // 0と等しくないかをalに格納
			printf("    movzx rax, al\n"); // alをゼロ拡張

			printf(".Lcond%d:\n", lend);
			return ;
		}
		case ND_BITWISE_AND:
		{
			gen(node->lhs);
			push();
			gen(node->rhs);
			pop(RDI);

			printf("   and %s, %s\n", RAX, RDI);
			return ;
		}
		case ND_BITWISE_XOR:
		{
			gen(node->lhs);
			push();
			gen(node->rhs);
			pop(RDI);

			printf("   xor %s, %s\n", RAX, RDI);
			return ;
		}
		case ND_BITWISE_OR:
		{
			gen(node->lhs);
			push();
			gen(node->rhs);
			pop(RDI);

			printf("    or %s, %s\n", RAX, RDI);
			return ;
		}
		case ND_SHIFT_RIGHT:
		{
			gen(node->rhs);
			push();
			gen(node->lhs);
			pop(RDI);
			mov(CL, DIL);

			printf("    shr %s, %s\n", RAX, CL);
			return;
		}
		case ND_SHIFT_LEFT:
		{
			gen(node->rhs);
			push();
			gen(node->lhs);
			pop(RDI);
			mov(CL, DIL);

			printf("    shl %s, %s\n", RAX, CL);
			return;
		}
		case ND_EQUAL:
		{
			equality_prologue(node);
			cmp(node->lhs->type);
			printf("    sete al\n");
			printf("    movzx rax, al\n");
			return;
		}
		case ND_NEQUAL:
		{
			equality_prologue(node);
			cmp(node->lhs->type);
			printf("    setne al\n");
			printf("    movzx rax, al\n");
			return;
		}
		case ND_LESS:
		{
			relational_prologue(node);
			cmp(node->lhs->type);
			printf("    setl al\n");
			printf("    movzx rax, al\n");
			return;
		}
		case ND_LESSEQ:
		{
			relational_prologue(node);
			cmp(node->lhs->type);
			printf("    setle al\n");
			printf("    movzx rax, al\n");
			return;
		}
		case ND_ADDR:
		{
			switch(node->lhs->kind)
			{
				// 変数ならアドレスを取り出す
				case ND_VAR_LOCAL:
				case ND_VAR_GLOBAL:
					lval(node->lhs);
					break ;
				// 構造体, unionからのアクセスなら、アドレスなのでそのまま返す
				case ND_MEMBER_VALUE:
				case ND_MEMBER_PTR_VALUE:
					arrow(node->lhs, true);
					break ;
				// ND_DEREFならアドレスで止める
				case ND_DEREF:
					gen(node->lhs->lhs);//ここ！！　↓
					break ;
				default:
					error("ND_ADDRを使えない kind:%d", node->lhs->kind);
					break ;
			}
			return;
		}
		case ND_DEREF:
		{
			gen(node->lhs);// ここと同じ
			load(node->type);
			return;
		}
		case ND_BITWISE_NOT:
		{
			gen(node->lhs);
			printf("    xor %s, -1\n", RAX);
			return;
		}
		case ND_MEMBER_VALUE:
		case ND_MEMBER_PTR_VALUE:
		{
			arrow(node, false);
			return;
		}
		case ND_CAST:
		{
			gen(node->lhs);
			cast(node->lhs->type, node->type);
			return;
		}
		case ND_VAR_LOCAL:
		{
			lval(node);
			load(node->type);
			return ;
		}
		case ND_VAR_GLOBAL:
		{
			lval(node);
			load(node->type);
			return;
		}
		case ND_STR_LITERAL:
		{
			printf("    %s %s, [rip + L_STR_%d]\n", ASM_LEA, RAX, node->def_str->index);
			return;
		}
		case ND_CALL:
		{
			call(node);
			return;
		}
		default:
		{
			error("不明なノード kind:%d type:%d", node->kind, node->type->ty);
			return;
		}
	}
}

static	t_lvar	*g_funcnow_locals;

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
		{
			mov(RAX, RBP);
			push();
			mov(RBP, RSP);
			g_funcnow_locals = NULL;
			return ;
		}
		case IL_FUNC_EPILOGUE:
		{
			mov(RSP, RBP);
			pop(RBP);
			printf("    ret\n");
			g_funcnow_locals = NULL;
			return ;
		}

		case IL_PUSH_NUM:
			pushi(code->number_int);
			return ;
		case IL_POP:
			// TODO 型
			pop(RAX);
			return ;

		case IL_DEF_VAR_LOCAL:
			// TODO 引数の配列型
			printf("    sub %s, %d\n", RSP, get_type_size(code->lvar->type));
			return ;
		case IL_DEF_VAR_END:
			// TODO アラインする
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

		case IL_ASSIGN:
			pop(RAX);
			pop(R10);
			// TODO 型を考慮する
			store_value(get_type_size(code->type));
			push();
			return ;

		case IL_VAR_LOCAL:
			code->kind = IL_VAR_LOCAL_ADDR;
			gen_il(code);
			code->kind = IL_VAR_LOCAL;
			pop(RAX);
			load(code->lvar->type);
			push();
			return ;

		case IL_VAR_LOCAL_ADDR:
			mov(RAX, RBP);
			if (code->lvar->offset > 0)
				printf("    sub %s, %d\n", RAX, code->lvar->offset);
			else if (code->lvar->offset < 0)
				printf("    add %s, %d\n", RAX, -code->lvar->offset);
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
